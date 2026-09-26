#!/usr/bin/env python3
"""Build the pm3 frames for the gen2 addressed-backdoor bench, from a UID as `hf 15 reader` prints it.

    tools/gen2-addressed-frames.py E0 07 81 B8 AF 14 42 07
    tools/gen2-addressed-frames.py E00781B8AF144207

THE QUESTION. Every gen2 backdoor frame this app sends is UNADDRESSED -- `02 E0 09 <ref> d0..d3`.
The reason given in the poller and the release notes is that 0xE0 is proprietary, so a conforming
tag rejects it on the command. That covers CONFORMING tags; another gen2 magic card in the field
parses 0xE0 0x09 exactly as the target does. Whether the backdoor's own parser honours the ISO15693
ADDRESSED flag is not a question the standard answers -- only the silicon does.

THE HAZARD, and it is why this tool exists rather than a hand-typed frame. If the parser IGNORES the
flag and reads positionally, `22 E0 <uid...>` is read as cmd=E0, sub=uid[7], ref=uid[6],
data=uid[5..2] -- a write to an arbitrary register. It is safe only while uid[7], the LAST byte
`hf 15 reader` prints, is not 0x09. This refuses to emit an addressed frame when it is.

Only block 0x40 (uid[7..4]) is ever written here. The CFG registers 0x47 and 0x52 carry block count,
block size and IC reference and are NOT touched: a wrong value there changes what the card reports
about itself, and `gen-2-card` is already on the shelf advertising 256 blocks against 64 physical
from an earlier fixture.
"""
import sys

SUB = 0x09
BLK_UID_7654 = 0x40  # takes uid[7], uid[6], uid[5], uid[4] IN THAT ORDER -- not reversed
BLK_CFG, BLK_CFG2 = 0x47, 0x52  # never written by this tool

# Asserted every run, against frames a card actually accepted.
# 1-2: the gen1 addressed bench, for the UID-on-the-wire order (LSB first), verbatim from
#      pr-round-15/addressed-writes-measured.md.
# 3:   the gold-tag result in NEXT-SESSION -- AA BB CC DD into a UID register moved the UID to
#      E0 48 03 00 DD CC BB AA, which is what fixes data byte 0 -> uid[7] rather than the reverse.
SELFTEST_UID = bytes.fromhex("E002222450008303")        # lri2k-keychain, as `hf 15 reader` prints
SELFTEST_WIRE = "03830050242202E0"                      # from `222103830050242202E00811223344`
SELFTEST_GOLD_BEFORE = bytes.fromhex("E048030001CDF136")
SELFTEST_GOLD_AFTER = bytes.fromhex("E0480300DDCCBBAA")  # after AA BB CC DD into its UID register


def wire(uid):
    """The address as it travels: least significant byte first, so uid[0] (0xE0) goes LAST."""
    return "".join("%02X" % b for b in reversed(uid))


def apply_7654(uid, data):
    """What the printed UID becomes after `data` lands in block 0x40."""
    out = bytearray(uid)
    out[7], out[6], out[5], out[4] = data[0], data[1], data[2], data[3]
    return bytes(out)


def selftest():
    bad = []
    if wire(SELFTEST_UID) != SELFTEST_WIRE:
        bad.append("wire order: %s != %s" % (wire(SELFTEST_UID), SELFTEST_WIRE))
    got = apply_7654(SELFTEST_GOLD_BEFORE, bytes.fromhex("AABBCCDD"))
    if got != SELFTEST_GOLD_AFTER:
        bad.append("uid[7..4] mapping: %s != %s" % (got.hex().upper(), SELFTEST_GOLD_AFTER.hex().upper()))
    if bad:
        print("SELFTEST FAILED against the measured transcripts; refusing to emit frames:")
        for b in bad:
            print("   " + b)
    return not bad


def frames(uid, probe=b"\xAA\xBB\xCC\xDD"):
    a = wire(uid)
    bad_a = a[:-2] + ("%02X" % (uid[0] ^ 0x01))       # one byte wrong, in the same place every
    orig = bytes([uid[7], uid[6], uid[5], uid[4]])     # prior bench flipped it: E0 -> E1
    p, o = probe.hex().upper(), orig.hex().upper()
    moved = apply_7654(uid, probe)
    print("  UID as printed      %s" % " ".join("%02X" % b for b in uid))
    print("  on the wire         %s   (least significant byte first)" % a)
    print("  block 0x40 holds    %s   <- restore value, uid[7] uid[6] uid[5] uid[4]" % o)
    if uid[7] == SUB:
        print()
        print("  *** REFUSING THE ADDRESSED FRAMES: uid[7] is 0x09. ***")
        print("  A parser that ignores the ADDRESSED flag would read that as the subcommand and")
        print("  write %02X %02X %02X %02X into register 0x%02X. Bench this card unaddressed only."
              % (uid[5], uid[4], uid[3], uid[2], uid[6]))
        return
    print()
    print("  1  positive control, the form the app sends today -- expect an answer, UID moves")
    print("     hf 15 raw -ackw -d 02E009%02X%s" % (BLK_UID_7654, p))
    print("     UID should become   %s" % " ".join("%02X" % b for b in moved))
    print("  2  restore, unaddressed -- always works, no re-address seam to worry about")
    print("     hf 15 raw -ackw -d 02E009%02X%s" % (BLK_UID_7654, o))
    print("  3  THE MEASUREMENT: addressed, correct UID, no OPTION")
    print("     hf 15 raw -ackw -d 22E0%s09%02X%s" % (a, BLK_UID_7654, p))
    print("  4  THE DISCRIMINATOR: addressed, UID one byte wrong -- must NOT move the UID")
    print("     hf 15 raw -ckw  -d 22E0%s09%02X%s" % (bad_a, BLK_UID_7654, p))
    print("  5  addressed + OPTION, correct UID   (TI silicon wants OPTION on standard writes)")
    print("     hf 15 raw -ackw -d 62E0%s09%02X%s" % (a, BLK_UID_7654, p))
    print("  6  unaddressed + OPTION, correct UID")
    print("     hf 15 raw -ackw -d 42E009%02X%s" % (BLK_UID_7654, p))
    print("  restore after ANY of 3-6 that moved it")
    print("     hf 15 raw -ackw -d 02E009%02X%s" % (BLK_UID_7654, o))


def main(argv):
    if not selftest():
        return 2
    if not argv:
        print(__doc__)
        return 1
    raw = "".join(argv).replace(" ", "")
    try:
        uid = bytes.fromhex(raw)
    except ValueError:
        print("not hex: %r" % raw)
        return 1
    if len(uid) != 8:
        print("need 8 UID bytes, got %d" % len(uid))
        return 1
    frames(uid)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
