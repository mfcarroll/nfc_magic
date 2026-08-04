#!/usr/bin/env python3
"""Generate synthetic ISO15693-3 source .nfc files for testing the nfc_magic ISO15693 clone/impersonation.

Each file is a valid Flipper "ISO15693-3" device dump with a chosen identity (UID / IC ref / DSFID /
AFI / block count) and data pattern. Drop them in the Flipper's `nfc/` folder, then in NFC Magic:
Check the magic target -> ISO15693 menu -> Write -> pick the file -> re-read the card and compare.

The set is chosen to exercise, on ONE magic target:
  slix_28              impersonate a SMALLER type (28-block ICODE SLIX). All blocks real -> clean.
  lri2k_56             impersonate a SMALLER type (56-block ST LRi2K).
  tagit_64             same size (64-block TI Tag-it), different IC ref / manufacturer.
  edgedata_64          64 blocks with NON-ZERO data in the last blocks (62/63) -- edge data that fits.
  oversize_edgedata_70 70 blocks with NON-ZERO data in blocks 64-69 -- edge data that CANNOT fit a
                       64-block target (should clone 0-63 and report the non-empty tail as beyond capacity).
  oversize_empty_70    70 blocks, tail EMPTY -- should clone cleanly and report a plain Success.

Usage:
    python3 tools/make_test_iso15693_nfc.py                 # -> tools/test_nfc/*.nfc
    python3 tools/make_test_iso15693_nfc.py --out ~/nfc     # write straight to a mounted SD `nfc/` dir
    python3 tools/make_test_iso15693_nfc.py --list
"""
import argparse
import os

# profile = dict(uid=8 bytes, ic_ref, dsfid, afi, blocks, data={blocknum: "AABBCCDD"} for non-zero blocks)
_HDR0, _HDR1 = "E3082156", "82186000"  # a plausible block0/block1 (mirrors a real read); rest zero
PROFILES = {
    "slix_28": dict(uid="E0 04 01 10 A1 A2 A3 A4", ic_ref=0x03, dsfid=0x00, afi=0x00, blocks=28,
                    data={0: _HDR0, 1: _HDR1},
                    note="NXP ICODE SLIX, 28 blocks -- impersonate a smaller type"),
    "lri2k_56": dict(uid="E0 02 08 B1 B2 B3 B4 B5", ic_ref=0x1A, dsfid=0x00, afi=0x00, blocks=56,
                     data={0: _HDR0, 1: _HDR1},
                     note="ST LRi2K, 56 blocks -- impersonate a smaller type"),
    "tagit_64": dict(uid="E0 07 C0 C1 C2 C3 C4 C5", ic_ref=0x00, dsfid=0x00, afi=0x00, blocks=64,
                     data={0: _HDR0, 1: _HDR1},
                     note="TI Tag-it, 64 blocks -- same size, different manufacturer/IC ref"),
    "edgedata_64": dict(uid="E0 04 01 10 D1 D2 D3 D4", ic_ref=0x0F, dsfid=0x02, afi=0x00, blocks=64,
                        data={0: _HDR0, 1: _HDR1, 62: "DEADBEEF", 63: "CAFEBABE"},
                        note="64 blocks, non-zero data in 62/63 -- edge data that FITS a 64-blk target"),
    "oversize_edgedata_70": dict(uid="E0 04 01 10 E1 E2 E3 E4", ic_ref=0x0F, dsfid=0x02, afi=0x00, blocks=70,
                                 data={0: _HDR0, 1: _HDR1,
                                       64: "11111111", 65: "22222222", 66: "33333333",
                                       67: "44444444", 68: "55555555", 69: "66666666"},
                                 note="70 blocks, non-zero data in 64-69 -- edge data that CANNOT fit a 64-blk target"),
    "oversize_empty_70": dict(uid="E0 04 01 10 F1 F2 F3 F4", ic_ref=0x0F, dsfid=0x02, afi=0x00, blocks=70,
                              data={0: _HDR0, 1: _HDR1},
                              note="70 blocks, empty tail -- over-reports but should clone cleanly (Success)"),
    # The only profile with a NON-ZERO AFI, and it exists because every other one has afi=0x00 -- which
    # makes the harness's AFI check vacuous. A card that silently refused every WRITE AFI still reads
    # back 0x00 and still compares equal, so "AFI OK 00->00" proves nothing. That is precisely the
    # failure the app's AFI/DSFID read-back verification was built to catch (a tag can refuse in-band,
    # answering with the error flag set in a well-formed frame, so the send's return value is not
    # evidence), and it had never been presented with an AFI it could fail on. DSFID is already covered
    # by the 0x02 profiles.
    "identity_64": dict(uid="E0 04 01 10 1D 1D 1D 1D", ic_ref=0x0F, dsfid=0x05, afi=0x27, blocks=64,
                        data={0: _HDR0, 1: _HDR1},
                        note="64 blocks, NON-ZERO AFI 0x27 + DSFID 0x05 -- the only real test of the AFI write"),
    # Wipe-residue seed for the merge gate. EVERY block carries 5A <blk> A5 <blk>: obviously synthetic,
    # self-identifying (so block-addressing weirdness shows up as a wrong number, not just wrong data),
    # and the same marker iso15693_magic_probe.py's writespan probe uses.
    #
    # Why a full fill is needed: every other 64-block profile here is zeros above block 1, so residue a
    # wipe failed to clear is indistinguishable from a wipe that worked. Clone this, then clone
    # slix_28 over it -- which reprograms the card to ADVERTISE 28 blocks -- then wipe. The wipe stops
    # at the advertised 28, so blocks 28..63 should still read back as 5A <blk> A5 <blk> while the
    # screen says the wipe succeeded.
    "wipeseed_64": dict(uid="E0 04 01 10 5E ED 00 01", ic_ref=0x0F, dsfid=0x00, afi=0x00, blocks=64,
                        data={b: "5A%02XA5%02X" % (b, b) for b in range(64)},
                        note="64 blocks, EVERY block 5A<blk>A5<blk> -- wipe-residue seed for the merge gate"),
}

_TYPES = ("ISO14443-3A, ISO14443-3B, ISO14443-4A, ISO14443-4B, ISO15693-3, FeliCa, NTAG/Ultralight, "
          "Mifare Classic, Mifare Plus, Mifare DESFire, SLIX, ST25TB, EMV")


def hexbytes(spaced_or_packed):
    return spaced_or_packed.replace(" ", "").upper()


def build_nfc(p):
    blocks, size = p["blocks"], 4
    data = bytearray(blocks * size)
    for blk, val in p.get("data", {}).items():
        b = bytes.fromhex(val)
        data[blk * size:blk * size + size] = b
    data_hex = " ".join("%02X" % x for x in data)
    sec_hex = " ".join("00" for _ in range(blocks))
    uid = " ".join("%02X" % int(hexbytes(p["uid"])[i:i + 2], 16) for i in range(0, 16, 2))
    return "\n".join([
        "Filetype: Flipper NFC device",
        "Version: 4",
        "# Device type can be %s" % _TYPES,
        "Device type: ISO15693-3",
        "# UID is common for all formats",
        "UID: %s" % uid,
        "# ISO15693-3 specific data",
        "# Data Storage Format Identifier",
        "DSFID: %02X" % p["dsfid"],
        "# Application Family Identifier",
        "AFI: %02X" % p["afi"],
        "# IC Reference - Vendor specific meaning",
        "IC Reference: %02X" % p["ic_ref"],
        "# Lock Bits",
        "Lock DSFID: false",
        "Lock AFI: false",
        "# Number of memory blocks, valid range = 1..256",
        "Block Count: %d" % blocks,
        "# Size of a single memory block, valid range = 01...20 (hex)",
        "Block Size: %02d" % size,
        "Data Content: %s" % data_hex,
        "# Block Security Status: 01 = locked, 00 = not locked",
        "Security Status: %s" % sec_hex,
        "",
    ])


def main():
    ap = argparse.ArgumentParser(description="Generate ISO15693-3 test .nfc files for ISO15693 clone testing.")
    ap.add_argument("--out", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_nfc"),
                    help="output dir (default: tools/test_nfc/). Point at a mounted SD `nfc/` to use directly.")
    ap.add_argument("--only", help="comma-list of profiles to emit (default: all). See --list.")
    ap.add_argument("--list", action="store_true")
    args = ap.parse_args()

    if args.list:
        print("Test profiles (name : blocks : IC ref : note):")
        for k, v in PROFILES.items():
            print("  %-22s %3d blk  IC 0x%02X  %s" % (k, v["blocks"], v["ic_ref"], v["note"]))
        return

    names = [n.strip() for n in args.only.split(",")] if args.only else list(PROFILES)
    os.makedirs(args.out, exist_ok=True)
    for name in names:
        if name not in PROFILES:
            raise SystemExit("unknown profile '%s' (see --list)" % name)
        path = os.path.join(args.out, "iso15693_%s.nfc" % name)
        with open(path, "w") as f:
            f.write(build_nfc(PROFILES[name]))
        print("wrote %s  (%d blocks, IC 0x%02X)" % (path, PROFILES[name]["blocks"], PROFILES[name]["ic_ref"]))
    print("\nCopy these to the Flipper's `nfc/` folder, then NFC Magic -> Check -> ISO15693 -> Write -> pick one.")


if __name__ == "__main__":
    main()
