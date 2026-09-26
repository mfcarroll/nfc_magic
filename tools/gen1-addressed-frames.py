#!/usr/bin/env python3
"""Build the pm3 frames for the gen1 addressed-write bench, from a UID as `hf 15 reader` prints it.

    tools/gen1-addressed-frames.py E0 02 22 24 50 00 83 03
    tools/gen1-addressed-frames.py E002222450008303

The UID travels LEAST SIGNIFICANT BYTE FIRST, which is the reverse of the order every screen, every
.nfc file and `hf 15 reader` print it in -- so uid[0], the 0xE0, is the LAST byte on the wire. Doing
that by hand is the error this exists to remove, and it does not fail loudly: a reversed address
names a card that is not in the field, so the write meets silence and reads as a card refusing
everything. Indistinguishable from the result the bench is looking for.

Prints the write, what the UID should become, the restore addressed to the MOVED uid, and the
mis-addressed control -- because an acceptance alone does not show the card MATCHED the address; a
tag ignoring the flag entirely answers identically (BENCH-RULE 2).
"""
import sys

BLK_UID_7654 = 0x38  # carries uid[7..4] -- the LAST four bytes `hf 15 reader` prints

# Asserted every run. Both lines are VERBATIM from pr-round-15/addressed-writes-measured.md, so the
# byte order is checked against a frame a card actually accepted rather than against my reading of
# the mapping. The restore line is the one that caught a reversed nibble order here.
SELFTEST_UID = bytes.fromhex("E002222450008303")          # lri2k-keychain, as `hf 15 reader` prints
SELFTEST_ADDR = "03830050242202E0"                        # from `222103830050242202E00811223344`
SELFTEST_RESTORE_DATA = "03830050"                        # from `02213803830050`


def wire(uid):
    return "".join("%02X" % b for b in reversed(uid))


def selftest():
    bad = []
    if wire(SELFTEST_UID) != SELFTEST_ADDR:
        bad.append("address: %s != %s" % (wire(SELFTEST_UID), SELFTEST_ADDR))
    restore = bytes(reversed(SELFTEST_UID[4:8])).hex().upper()
    if restore != SELFTEST_RESTORE_DATA:
        bad.append("restore data: %s != %s" % (restore, SELFTEST_RESTORE_DATA))
    if bad:
        print("SELFTEST FAILED against the measured transcript; refusing to emit frames:")
        for b in bad:
            print("   " + b)
    return not bad


def main(argv):
    if not selftest():
        return 2
    raw = "".join(argv).replace(":", "").replace("-", "")
    try:
        uid = bytes.fromhex(raw)
    except ValueError:
        print(__doc__)
        return 2
    if len(uid) != 8:
        print("error: a UID is 8 bytes, got %d" % len(uid))
        return 2

    test = bytes([0xAA, 0xBB, 0xCC, 0xDD])
    # Block 56's DATA is uid[7],uid[6],uid[5],uid[4] -- data[0] lands in uid[7], the LAST byte
    # printed. So the four bytes that restore it are uid[4:8] REVERSED, not uid[4:8]. Getting this
    # backwards was the first thing this script did, which is the whole argument for it existing.
    orig_hi = bytes(reversed(uid[4:8]))
    moved = uid[0:4] + bytes(reversed(test))

    print("UID as printed :  " + " ".join("%02X" % b for b in uid))
    print("on the wire    :  " + wire(uid) + "   (least significant byte first)")
    print()
    print("  hf 15 reader                                       expect the UID above")
    print("  hf 15 raw -ackw -d 2221%s%02X%s"
          % (wire(uid), BLK_UID_7654, test.hex().upper()))
    print("      PREDICTION: 00 78 F0, accepted")
    print("  hf 15 reader                                       expect "
          + " ".join("%02X" % b for b in moved))
    print()
    print("  # the mis-addressed control, against the MOVED uid -- one byte wrong, expect SILENCE")
    bad = bytearray(moved)
    bad[0] ^= 0x01
    print("  hf 15 raw -ackw -d 2221%s%02X%s"
          % (wire(bytes(bad)), BLK_UID_7654, test.hex().upper()))
    print("  hf 15 reader                                       expect "
          + " ".join("%02X" % b for b in moved) + "  (brackets the silence)")
    print()
    print("  # restore, addressed to the MOVED uid -- this is the re-address seam in miniature")
    print("  hf 15 raw -ackw -d 2221%s%02X%s"
          % (wire(moved), BLK_UID_7654, orig_hi.hex().upper()))
    print("  hf 15 reader                                       expect "
          + " ".join("%02X" % b for b in uid) + "  (back where it started)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
