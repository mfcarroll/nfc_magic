#!/usr/bin/env python3
"""Goal-2: ground-truth the nfc_magic app's ISO15693 / SLIX clone against real hardware.

This is the human-in-the-loop other half of the proxmark harness (iso15693_magic_probe.py). The
actual card write happens on the Flipper's own UI (we can't and shouldn't automate the on-device
NFC Magic app); this tool automates everything around it so a run is fast and repeatable:

  for each source .nfc (default tools/test_nfc/slixtest_*.nfc):
    1. parse the source locally (UID / geometry / IC / DSFID / AFI / block data)
    2. UPLOAD it to the Flipper's nfc/ folder  (prefixed `gt_` so it's easy to spot / clean up)
    3. snapshot the nfc/ file list, then print on-device instructions and WAIT for the operator to
         - write the source onto the magic card with the NFC Magic app, then
         - read that card back with the stock NFC app and Save it (makes a new nfc/ file)
    4. snapshot again -> the NEW file is the read-back; DOWNLOAD it
    5. COMPARE source vs read-back: identity (UID/IC/DSFID/AFI), block-by-block data, and
       over-capacity (source blocks the physically-smaller card couldn't hold) -- this is exactly
       what validates the app's clone + honest-partial behaviour.
    6. (optional) cross-read the same card with a Proxmark3 as an independent second opinion.

Everything lands in a campaign dir (campaign.log / manifest.json / raw/) just like the pm3 harness.

Requires pyserial -> run under the tools venv:
    tools/.venv/bin/python tools/flipper_ground_truth.py --sources tools/test_nfc/slixtest_*.nfc
"""

import argparse
import difflib
import glob
import json
import os
import sys
from datetime import datetime

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from flipper_bridge import Flipper, FlipperBridgeError, new_files  # noqa: E402

# --------------------------------------------------------------------------- colour
_USE_COLOUR = sys.stdout.isatty() and os.environ.get("NO_COLOR") is None
_PAL = {"head": "1;36", "ok": "1;32", "warn": "1;33", "err": "1;31",
        "dim": "2", "flip": "1;35", "step": "1;34"}


def C(kind, s):
    if not _USE_COLOUR:
        return s
    return "\033[%sm%s\033[0m" % (_PAL.get(kind, "0"), s)


def hr(ch="=", n=78):
    return ch * n


# --------------------------------------------------------------------------- .nfc parse
def parse_nfc(text):
    """Parse a Flipper NFC device file (ISO15693-3 / SLIX). Tolerant of unknown keys."""
    d = {"device_type": None, "uid": None, "dsfid": None, "afi": None, "ic_ref": None,
         "block_count": None, "block_size": None, "data_hex": None, "security_hex": None,
         "extra": {}}
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or ":" not in line:
            continue
        key, _, val = line.partition(":")
        key, val, kl = key.strip(), val.strip(), key.strip().lower()
        if kl == "device type":
            d["device_type"] = val
        elif kl == "uid":
            d["uid"] = val.upper()
        elif kl == "dsfid":
            d["dsfid"] = val.upper()
        elif kl == "afi":
            d["afi"] = val.upper()
        elif kl == "ic reference":
            d["ic_ref"] = val.upper()
        elif kl == "block count":
            d["block_count"] = _int(val, 10)
        elif kl == "block size":
            d["block_size"] = _int(val, 16)  # file comment: "valid range = 01...20 (hex)"
        elif kl == "data content":
            d["data_hex"] = val.upper()
        elif kl == "security status":
            d["security_hex"] = val.upper()
        else:
            d["extra"][key] = val
    return d


def _int(val, base):
    try:
        return int(val, base)
    except (ValueError, TypeError):
        return None


def blocks_of(p):
    """Split 'Data Content' into a list of per-block hex strings ('AA BB CC DD')."""
    if not p.get("data_hex") or not p.get("block_size"):
        return []
    toks = p["data_hex"].split()
    bs = p["block_size"]
    return [" ".join(toks[i:i + bs]) for i in range(0, len(toks), bs)]


def _is_zero(block_hex):
    return set(block_hex.replace(" ", "")) <= {"0"}


# --------------------------------------------------------------------------- compare
def compare(src, read):
    sb, rb = blocks_of(src), blocks_of(read)
    overlap = min(len(sb), len(rb))
    mism = [{"block": i, "src": sb[i], "read": rb[i]} for i in range(overlap) if sb[i] != rb[i]]
    overflow = sb[len(rb):]  # source blocks the card had no room for
    overflow_nonempty = [{"block": len(rb) + i, "src": b} for i, b in enumerate(overflow) if not _is_zero(b)]
    r = {
        "uid_match": src.get("uid") == read.get("uid"),
        "ic_match": src.get("ic_ref") == read.get("ic_ref"),
        "dsfid_match": src.get("dsfid") == read.get("dsfid"),
        "afi_match": src.get("afi") == read.get("afi"),
        "src_blocks": src.get("block_count"),
        "read_blocks": read.get("block_count"),
        "overlap_blocks": overlap,
        "data_mismatches": mism,
        "src_overflow_blocks": len(overflow),
        "src_overflow_nonempty": overflow_nonempty,
        "read_extra_blocks": max(0, len(rb) - len(sb)),
    }
    r["identity_ok"] = all((r["uid_match"], r["ic_match"], r["dsfid_match"], r["afi_match"]))
    r["data_ok"] = len(mism) == 0
    return r


def verdict_line(r):
    if r["identity_ok"] and r["data_ok"] and r["src_overflow_blocks"] == 0:
        return C("ok", "PASS -- byte-identical clone (identity + all blocks)")
    if r["identity_ok"] and r["data_ok"] and not r["src_overflow_nonempty"]:
        return C("ok", "PASS -- identity + all storable blocks match; %d empty source block(s) "
                       "beyond the card (expected)" % r["src_overflow_blocks"])
    bits = []
    bits.append("identity " + ("ok" if r["identity_ok"] else "MISMATCH"))
    if r["data_mismatches"]:
        bits.append("%d block mismatch(es)" % len(r["data_mismatches"]))
    if r["src_overflow_nonempty"]:
        bits.append("%d NON-EMPTY source block(s) the card couldn't hold" % len(r["src_overflow_nonempty"]))
    return C("warn", "PARTIAL -- " + "; ".join(bits))


# --------------------------------------------------------------------------- proxmark (optional)
def pm3_crosscheck(pm3, split, src, save_raw):
    """Independent second read of the just-written card via Proxmark3 (identity + geometry only)."""
    from iso15693_magic_probe import pm15_info_retry  # lazy: pm3 path is optional
    info, raw = pm15_info_retry(pm3, split)
    save_raw("pm3_info", raw)
    got_uid = (info.get("uid") or "").replace(" ", "").upper()
    want_uid = (src.get("uid") or "").replace(" ", "").upper()
    return {
        "uid": info.get("uid"), "uid_match": got_uid == want_uid,
        "block_count": info.get("block_count"), "block_count_match": info.get("block_count") == src.get("block_count"),
        "ic_ref": info.get("ic_ref"),
    }


# --------------------------------------------------------------------------- run
def main():
    ap = argparse.ArgumentParser(
        description="Ground-truth the nfc_magic ISO15693/SLIX clone via Flipper upload + read-back compare.",
        formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    ap.add_argument("--sources", nargs="+", help="source .nfc files (default: tools/test_nfc/slixtest_*.nfc)")
    ap.add_argument("--port", default="auto", help="Flipper serial port (default: auto-detect)")
    ap.add_argument("--scripts", default=None, help="path to Flipper firmware `scripts` dir")
    ap.add_argument("--out", default=None, help="campaign output dir (default: tools/campaigns/gt_<stamp>)")
    ap.add_argument("--prefix", default="gt_", help="prefix for uploaded source files (default: gt_)")
    ap.add_argument("--keep-uploaded", action="store_true", help="leave the uploaded gt_ file on the Flipper")
    ap.add_argument("--manual-readback", default=None,
                    help="skip new-file detection; use this exact nfc/ filename as the read-back")
    ap.add_argument("--pm3-crosscheck", action="store_true", help="also read the card with a Proxmark3")
    ap.add_argument("--pm3", default="pm3", help="proxmark client binary (with --pm3-crosscheck)")
    ap.add_argument("--pm3-split", action="store_true", help="pass commands to pm3 split (with --pm3-crosscheck)")
    ap.add_argument("--dry-run", action="store_true", help="parse + preview only; no Flipper, no prompts")
    args = ap.parse_args()

    sources = []
    for pat in (args.sources or ["tools/test_nfc/slixtest_*.nfc"]):
        sources.extend(sorted(glob.glob(pat)) if any(c in pat for c in "*?[") else [pat])
    sources = [s for s in sources if os.path.isfile(s)]
    if not sources:
        print(C("err", "no source .nfc files found (looked for tools/test_nfc/slixtest_*.nfc)."))
        return 2

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out_dir = args.out or os.path.join("tools", "campaigns", "gt_%s" % stamp)
    log_f = None
    if not args.dry_run:
        os.makedirs(os.path.join(out_dir, "raw"), exist_ok=True)
        log_f = open(os.path.join(out_dir, "campaign.log"), "w")
    manifest = {"stamp": stamp, "sources": [], "results": []}

    def wlog(s=""):
        print(s)
        if log_f:
            log_f.write(s + "\n")
            log_f.flush()

    def save_manifest():
        if not args.dry_run:
            with open(os.path.join(out_dir, "manifest.json"), "w") as f:
                json.dump(manifest, f, indent=2)

    def raw_saver(tag):
        def _save(sub, text):
            if args.dry_run:
                return
            fn = os.path.join(out_dir, "raw", "%s_%s.txt" % (tag, sub))
            with open(fn, "w") as f:
                f.write(text if isinstance(text, str) else str(text))
        return _save

    wlog(C("head", "=== ISO15693/SLIX Flipper ground-truth  %s ===" % stamp))
    wlog("sources : %s" % ", ".join(os.path.basename(s) for s in sources))
    wlog("output  : %s" % (out_dir if not args.dry_run else "(dry-run, none)"))

    flipper = None
    try:
        if not args.dry_run:
            flipper = Flipper(port=args.port, scripts=args.scripts).__enter__()
            wlog(C("ok", "Flipper connected on %s" % flipper.port))

        for src_path in sources:
            base = os.path.basename(src_path)
            with open(src_path) as f:
                src_txt = f.read()
            src = parse_nfc(src_txt)
            rec = {"source": base, "source_summary": {
                "device_type": src["device_type"], "uid": src["uid"], "ic_ref": src["ic_ref"],
                "dsfid": src["dsfid"], "afi": src["afi"],
                "block_count": src["block_count"], "block_size": src["block_size"]}}
            manifest["sources"].append(base)

            wlog("\n" + C("head", hr()))
            wlog(C("head", "  SOURCE: %s" % base))
            wlog(C("head", hr()))
            wlog("   device type : %s" % src["device_type"])
            wlog("   UID.........: %s" % src["uid"])
            wlog("   identity....: IC %s   DSFID %s   AFI %s" % (src["ic_ref"], src["dsfid"], src["afi"]))
            wlog("   geometry....: %s blocks x %s bytes" % (src["block_count"], src["block_size"]))

            if args.dry_run:
                wlog(C("dim", "   (dry-run: would upload as %s%s and prompt for a read-back)" % (args.prefix, base)))
                continue

            up_name = args.prefix + base
            flipper.upload(src_path, up_name)
            wlog(C("ok", "   uploaded -> nfc/%s" % up_name))
            before = flipper.list_nfc()

            # ---- on-device instructions ----
            wlog("")
            wlog(C("step", "   STEP 1  (Flipper -> NFC Magic):"))
            wlog("     Check Magic Tag -> place the MAGIC target card -> when detected, open the card")
            wlog("     menu -> Write -> pick  '%s'  -> confirm. Wait for 'Success' (or 'Clone partial ...')." % up_name)
            wlog(C("step", "   STEP 2  (Flipper -> stock NFC app):"))
            wlog("     Back to main menu -> NFC -> Read -> place the SAME card -> when read, Save")
            wlog("     (accept the default name). This drops a new file into nfc/.")
            if args.pm3_crosscheck:
                wlog(C("step", "   STEP 3  leave the card on the Proxmark3 antenna afterwards for the cross-read."))
            try:
                input(C("flip", "   >> do STEP 1 and STEP 2, then press Enter here (Ctrl-C aborts)... "))
            except (EOFError, KeyboardInterrupt):
                wlog(C("warn", "   skipped by operator."))
                rec["result"] = {"skipped": True}
                manifest["results"].append(rec)
                save_manifest()
                continue

            # ---- detect the read-back file ----
            if args.manual_readback:
                readback = args.manual_readback
            else:
                after = flipper.list_nfc()
                fresh = new_files(before, after, ignore={up_name})
                if len(fresh) == 1:
                    readback = fresh[0]
                elif len(fresh) == 0:
                    wlog(C("err", "   no new file appeared in nfc/. Did the stock app Save?"))
                    manual = input("     type the read-back filename (blank to skip): ").strip()
                    if not manual:
                        rec["result"] = {"error": "no read-back file"}
                        manifest["results"].append(rec)
                        save_manifest()
                        continue
                    readback = manual
                else:
                    wlog(C("warn", "   multiple new files: %s" % ", ".join(fresh)))
                    for i, n in enumerate(fresh):
                        wlog("       [%d] %s" % (i, n))
                    pick = input("     pick index (default newest = %d): " % (len(fresh) - 1)).strip()
                    readback = fresh[int(pick)] if pick.isdigit() and int(pick) < len(fresh) else fresh[-1]
            rec["readback_file"] = readback
            wlog(C("ok", "   read-back  <- nfc/%s" % readback))

            # ---- download + compare ----
            local_rb = os.path.join(out_dir, "raw", "readback_%s" % readback.replace("/", "_"))
            rb_txt = flipper.read_text(readback)
            with open(local_rb, "w") as f:
                f.write(rb_txt)
            read = parse_nfc(rb_txt)
            cmp = compare(src, read)
            rec["compare"] = cmp

            wlog("   ---- compare (source vs read-back) ----")
            wlog("   UID  %s   %s -> %s" % (_mk(cmp["uid_match"]), src["uid"], read["uid"]))
            wlog("   IC   %s   %s -> %s      DSFID %s %s->%s     AFI %s %s->%s"
                 % (_mk(cmp["ic_match"]), src["ic_ref"], read["ic_ref"],
                    _mk(cmp["dsfid_match"]), src["dsfid"], read["dsfid"],
                    _mk(cmp["afi_match"]), src["afi"], read["afi"]))
            wlog("   geometry  source %s blk / read %s blk  (overlap %s)"
                 % (cmp["src_blocks"], cmp["read_blocks"], cmp["overlap_blocks"]))
            if cmp["data_mismatches"]:
                wlog(C("warn", "   %d block mismatch(es):" % len(cmp["data_mismatches"])))
                for m in cmp["data_mismatches"][:8]:
                    wlog("       block %-3d src[%s] != read[%s]" % (m["block"], m["src"], m["read"]))
                if len(cmp["data_mismatches"]) > 8:
                    wlog("       ... and %d more" % (len(cmp["data_mismatches"]) - 8))
            if cmp["src_overflow_blocks"]:
                tail = "%d source block(s) beyond the card's %s" % (cmp["src_overflow_blocks"], cmp["read_blocks"])
                if cmp["src_overflow_nonempty"]:
                    wlog(C("warn", "   %s -- %d NON-EMPTY (data lost): %s"
                           % (tail, len(cmp["src_overflow_nonempty"]),
                              ", ".join("blk %d" % o["block"] for o in cmp["src_overflow_nonempty"][:8]))))
                else:
                    wlog(C("dim", "   %s -- all empty (expected over-capacity, no data lost)" % tail))
            wlog("   VERDICT: " + verdict_line(cmp))

            # unified diff of the two files (comment lines aside)
            diff = "\n".join(difflib.unified_diff(
                src_txt.splitlines(), rb_txt.splitlines(),
                fromfile="source/%s" % base, tofile="readback/%s" % readback, lineterm=""))
            with open(os.path.join(out_dir, "raw", "diff_%s.txt" % base), "w") as f:
                f.write(diff)

            # ---- optional proxmark cross-read ----
            if args.pm3_crosscheck:
                try:
                    pmc = pm3_crosscheck(args.pm3, args.pm3_split, src, raw_saver(base))
                    rec["pm3"] = pmc
                    wlog("   pm3 cross-read: UID %s %s   block_count %s %s   IC 0x%s"
                         % (_mk(pmc["uid_match"]), pmc["uid"], _mk(pmc["block_count_match"]),
                            pmc["block_count"], format(pmc["ic_ref"], "02X") if isinstance(pmc["ic_ref"], int) else "?"))
                except Exception as e:
                    wlog(C("err", "   pm3 cross-read error: %s" % e))
                    rec["pm3"] = {"error": str(e)}

            if not args.keep_uploaded:
                try:
                    flipper.remove(up_name)
                except Exception:
                    pass
            manifest["results"].append(rec)
            save_manifest()
    except FlipperBridgeError as e:
        print(C("err", "\nFlipper error: %s" % e))
        return 1
    except KeyboardInterrupt:
        wlog(C("warn", "\n[aborted by operator]"))
    finally:
        if flipper:
            flipper.__exit__(None, None, None)
        save_manifest()
        if log_f:
            log_f.close()

    if not args.dry_run:
        wlog("\n" + C("ok", "Done. Campaign dir: %s" % out_dir))
        wlog("  campaign.log / manifest.json / raw/*  (readbacks, diffs)")
    return 0


def _mk(ok):
    return C("ok", "OK ") if ok else C("err", "XX ")


if __name__ == "__main__":
    sys.exit(main())
