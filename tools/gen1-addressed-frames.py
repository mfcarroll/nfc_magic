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
SELFTEST_BLK57_DATA = "242202E0"                          # from `hf 15 wrbl --ua -b 57 -d 242202E0`
SELFTEST_TI_UID = bytes.fromhex("E007803DE2E73A29")       # white-coin, TI Tag-it HF-I Plus
SELFTEST_TI_ADDR = "293AE7E23D8007E0"                     # from `6221293AE7E23D8007E00855667788`

BLK_UID_3210 = 0x39  # carries uid[3..0] -- the FIRST four bytes `hf 15 reader` prints
OPTION_ADDRESSED = 0x62  # SUBCARRIER_1 | DATA_RATE_HI | T4_ADDRESSED | T4_OPTION


def wire(uid):
    return "".join("%02X" % b for b in reversed(uid))


def selftest():
    bad = []
    if wire(SELFTEST_UID) != SELFTEST_ADDR:
        bad.append("address: %s != %s" % (wire(SELFTEST_UID), SELFTEST_ADDR))
    restore = bytes(reversed(SELFTEST_UID[4:8])).hex().upper()
    if restore != SELFTEST_RESTORE_DATA:
        bad.append("block 56 data: %s != %s" % (restore, SELFTEST_RESTORE_DATA))
    blk57 = bytes(reversed(SELFTEST_UID[0:4])).hex().upper()
    if blk57 != SELFTEST_BLK57_DATA:
        bad.append("block 57 data: %s != %s" % (blk57, SELFTEST_BLK57_DATA))
    if wire(SELFTEST_TI_UID) != SELFTEST_TI_ADDR:
        bad.append("TI address: %s != %s" % (wire(SELFTEST_TI_UID), SELFTEST_TI_ADDR))
    if bad:
        print("SELFTEST FAILED against the measured transcript; refusing to emit frames:")
        for b in bad:
            print("   " + b)
    return not bad


def option_probe(uid, block=8):
    """Does a card that REQUIRES the OPTION flag also enforce the ADDRESS?

    The one gap left in the addressed-write table. It was skipped for a reason that was withdrawn
    the same day -- "TI refuses unaddressed writes, so it already discriminates" -- and never re-run.

    PM3 GETS AN ANSWER HERE EVEN THOUGH THE APP CANNOT. With OPTION set the card owes its reply only
    after a standalone EOF; the Flipper SDK has no call for one, which is why the app reads the block
    back instead. proxmark sends it (SendDataTagEOF), and the measured frame below proves it --
    6221...55667788 returned 00 78 F0 on this card. So on pm3 the RESPONSE discriminates, and the
    read-back is corroboration rather than the measurement.

    The correctly-addressed write has to be in the set, not assumed: without it a silence at step 1
    could equally be a malformed frame, which is a control that cannot fail (BENCH-RULE 2).
    """
    bad = bytearray(uid)
    bad[0] ^= 0x01
    data = "AABBCCDD"
    print("  hf 15 reader                       expect " + " ".join("%02X" % b for b in uid))
    print("  hf 15 rdbl -b %d                    RECORD what it holds -- the restore needs it" % block)
    print()
    print("  # 1. WRONG address, OPTION set")
    print("  hf 15 raw -ackw -d %02X21%s%02X%s" % (OPTION_ADDRESSED, wire(bytes(bad)), block, data))
    print("      PREDICTION: silence / command failed, if the address is enforced")
    print("  hf 15 reader                       brackets the silence: the card is still there")
    print()
    print("  # 2. RIGHT address, OPTION set -- the control, and it must be able to fail")
    print("  hf 15 raw -ackw -d %02X21%s%02X%s" % (OPTION_ADDRESSED, wire(uid), block, data))
    print("      PREDICTION: 00 78 F0, as the same frame shape measured on this card")
    print("  hf 15 rdbl -b %d                    corroboration: now AA BB CC DD" % block)
    print()
    print("  # 3. restore what step 0's read showed")
    print("  hf 15 raw -ackw -d %02X21%s%02X<original>" % (OPTION_ADDRESSED, wire(uid), block))


def enforce(uid, holds, option, block=8):
    """Does a mis-addressed write LAND? Silence does not answer that, and until 2026-09-26 nothing
    on this bench distinguished the two.

    Every enforcement control here but one rests on the card not ANSWERING a wrong address. That was
    the same thing as not writing until `42E0...` -- the gen2 backdoor with OPTION set -- reported
    failure and moved the UID anyway. That is a FLAG effect rather than an ADDRESS effect, so the
    inference still holds; it is just an inference on five of six cards.

    THE TWO THINGS THE EARLIER CONTROLS EACH MISSED, and both are needed:
      - the probe data must DIFFER from what the block already holds, or a landed write and a
        refused one read identically. `white-coin`'s control sent AABBCCDD in both frames.
      - the read must come BEFORE any restore. `black-tag`'s came after, so it confirmed the
        restore and nothing else.

    The right-address write goes AFTER the wrong-address one rather than before: it proves the frame
    shape was good, so a silence at step 1 was the ADDRESS and not a malformed frame (BENCH-RULE 2,
    a control that cannot fail is not a control).
    """
    flags = OPTION_ADDRESSED if option else 0x22
    bad = bytearray(uid)
    bad[0] ^= 0x01
    holds = holds.upper().replace(" ", "")
    probe = "55667788" if holds != "55667788" else "A1B2C3D4"
    print("  # 0. the card, and what block %d holds RIGHT NOW" % block)
    print("  hf 15 reader                     expect " + " ".join("%02X" % b for b in uid))
    print("  hf 15 rdbl -b %d                  this run assumes it holds %s" % (block, holds))
    print()
    print("  # 1. WRONG address, data that DIFFERS from what is in there")
    print("  hf 15 raw -ckw  -d %02X21%s%02X%s" % (flags, wire(bytes(bad)), block, probe))
    print("      PREDICTION: silence")
    print()
    print("  # 2. *** THE MEASUREMENT *** -- did it write anyway?")
    print("  hf 15 rdbl -b %d" % block)
    print("      %s   the wrong address wrote NOTHING -- enforcement MEASURED" % holds)
    print("      %s   IT WROTE. Stop and report: the round's safety claim rests on this" % probe)
    print()
    print("  # 3. RIGHT address, same data -- the control, and it must be able to fail")
    print("  hf 15 raw -ackw -d %02X21%s%02X%s" % (flags, wire(uid), block, probe))
    print("      PREDICTION: 00 78 F0. Proves the frame shape was good, so step 1 was the ADDRESS")
    print("  hf 15 rdbl -b %d                  expect %s" % (block, probe))
    print()
    print("  # 4. restore, and confirm")
    print("  hf 15 raw -ackw -d %02X21%s%02X%s" % (flags, wire(uid), block, holds))
    print("  hf 15 rdbl -b %d                  expect %s" % (block, holds))


def restore(uid):
    """The two UNADDRESSED backdoor writes that set a whole UID, as proxmark's own wipe path sends
    them. Unaddressed because pm3's wrbl --ua is what the earlier restores used and what the cards
    are measured taking -- and because a card whose UID is being repaired may be sharing that UID
    with another on the bench, which is precisely when an ADDRESS is no use.

    Block 56 carries uid[7..4] and block 57 uid[3..0], each in frame order, so both payloads are a
    printed half REVERSED. Two different reversals from the address form, which is why this is
    generated rather than typed."""
    print("  # ONE CARD ON THE ANTENNA. These frames are UNADDRESSED: every tag in the field takes")
    print("  # them, and that is the point here -- but it means a second card gets this UID too.")
    print("  hf 15 reader")
    print("  hf 15 wrbl --ua -b 56 -d %s" % bytes(reversed(uid[4:8])).hex().upper())
    print("  hf 15 wrbl --ua -b 57 -d %s" % bytes(reversed(uid[0:4])).hex().upper())
    print("  hf 15 reader                     expect " + " ".join("%02X" % b for b in uid))


def main(argv):
    if not selftest():
        return 2
    mode = "probe"
    argv = list(argv)
    option = "--option" in argv
    argv = [a for a in argv if a != "--option"]
    holds = "00000000"
    if "--holds" in argv:
        i = argv.index("--holds")
        holds = argv[i + 1]
        del argv[i:i + 2]
    if argv and argv[0] in ("--restore", "--probe", "--option-probe", "--enforce"):
        mode, argv = argv[0][2:], argv[1:]
    raw = "".join(argv).replace(":", "").replace("-", "")
    try:
        uid = bytes.fromhex(raw)
    except ValueError:
        print(__doc__)
        return 2
    if len(uid) != 8:
        print("error: a UID is 8 bytes, got %d" % len(uid))
        return 2

    if mode == "enforce":
        print("card          :  " + " ".join("%02X" % b for b in uid)
              + ("   OPTION set" if option else "   OPTION clear"))
        enforce(uid, holds, option)
        return 0

    if mode == "option-probe":
        print("card          :  " + " ".join("%02X" % b for b in uid))
        option_probe(uid)
        return 0

    if mode == "restore":
        print("restore to    :  " + " ".join("%02X" % b for b in uid))
        restore(uid)
        return 0

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
