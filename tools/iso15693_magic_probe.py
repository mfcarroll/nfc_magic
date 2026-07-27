#!/usr/bin/env python3
"""ISO15693 magic-card characterization harness -- probe an (often unknown) magic NfcV tag with a
Proxmark3, guided, and record EVERYTHING with metadata into a timestamped campaign dir.

This is the ISO15693 sibling of the T5577 capture campaign. Where that one sweeps LF configs, this
one answers the questions you actually have about a *magic* 15693 card before/after cloning with the
nfc_magic app:

  info        (safe)        `hf 15 info`  -> UID, chip TYPE, IC ref, DSFID, AFI, reported block geometry
  capacity    (safe)        read blocks upward until a read FAILS -> the PHYSICAL block count, and how
                            many the card OVER-reports (the "fake-flash" phantom tail). Reads only.
  magictype   (safe + opt)  V3 config-mode signature read (blocks 0x14/0x15); with --destructive also
                            tries gen1 and gen2 `csetuid` to see which UID-write the card accepts, then
                            restores the original UID.
  edgepages   (--destructive)  write non-zero to the last real block and the first phantom block, read
                            back, and check for aliasing (does writing block N wrap onto block 0?).
                            Restores the block afterwards.
  impersonate (--destructive)  send custom gen2 CFG frames (`hf 15 raw`) to make the card report other
                            geometries / IC refs (28-block SLIX, 56-block LRi2K, oversized, ...), and
                            check via `hf 15 info` whether it ACCEPTS or CLAMPS each. Restores geometry.

Non-destructive probes (info, capacity, the V3 signature read) never write. Destructive probes are
opt-in with --destructive, snapshot what they touch, and best-effort restore -- but treat a card you
care about as at-risk and use a blank first.

Outputs a campaign dir: human `campaign.log`, machine `manifest.json`, and `raw/*.txt` with the exact
pm3 output of every command (self-labelled with card/probe/timestamp/fw-commit) so nothing is ambiguous.

Examples:
    # full safe characterization of one card:
    python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes info,capacity,magictype

    # deep test on a BLANK magic card (writes!):
    python3 tools/iso15693_magic_probe.py --card blank1 --probes info,capacity,magictype,edgepages,impersonate --destructive

    # sweep several cards, safe probes, note the Flipper's reading of each for the proxmark-vs-Flipper diff:
    python3 tools/iso15693_magic_probe.py --card "card-a,card-b,card-c" --probes info,capacity --flipper-note

    python3 tools/iso15693_magic_probe.py --list-probes
    python3 tools/iso15693_magic_probe.py --dry-run --card x --probes info,capacity,magictype,edgepages,impersonate --destructive

Requires: Proxmark3 `pm3` client on PATH (or PM3=/path). No pip needed.
"""
import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from datetime import datetime

HERE = os.path.dirname(os.path.abspath(__file__))
_ANSI = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")

# Arbitrary valid E0-prefixed UIDs used only to test which magic UID-write a card accepts.
TEST_UID_GEN1 = "E0F1E2D3C4B5A697"
TEST_UID_GEN2 = "E0A1B2C3D4E5F607"
# Identity profiles the impersonate probe tries to make the card advertise (blocks, block_size, ic_ref).
IMPERSONATE_TARGETS = [
    ("SLIX-28", 28, 4, 0x01),
    ("LRi2K-56", 56, 4, 0x02),
    ("default-64", 64, 4, 0x8B),
    ("oversize-100", 100, 4, 0x8B),
]
# V3 (un-finalized) magic config-mode signature, blocks 0x14 / 0x15.
V3_SIG_A = "A5 2B 44 2C"
V3_SIG_B = "21 AE 93 00"


# =============================================================== console colour (same scheme as T5577)
class Colour:
    CODES = {"ok": "1;32", "err": "1;31", "warn": "1;33", "info": "1;36", "head": "1;35", "dim": "2",
             "pm3": "1;35", "flip": "1;36", "probe": "1;34"}

    def __init__(self, enabled=False):
        self.enabled = enabled

    def __call__(self, kind, s):
        code = self.CODES.get(kind)
        return "\033[%sm%s\033[0m" % (code, s) if (self.enabled and code) else s


C = Colour(False)


# =============================================================== Proxmark plumbing
PM3_FAIL_MARKERS = ["claimed by another process", "could not open", "waiting for proxmark3",
                    "failed to open", "no proxmark3 found", "permission denied", "resource busy",
                    "unable to open", "timed out"]


def pm3_failure_reason(text):
    t = (text or "").lower()
    for m in PM3_FAIL_MARKERS:
        if m in t:
            return m
    if not t.strip():
        return "no output (Proxmark3 client did not connect)"
    return None


def pm3_exec(pm3_bin, cmds, split=False, timeout=90):
    """Run pm3 command(s); return combined stdout+stderr. Batched with ';' unless split."""
    base = shlex.split(pm3_bin)
    out = ""
    groups = [[c] for c in cmds] if split else [cmds]
    for g in groups:
        joined = " ; ".join(g)
        try:
            r = subprocess.run(base + ["-c", joined], capture_output=True, text=True, timeout=timeout)
            out += (r.stdout or "") + (r.stderr or "")
        except subprocess.TimeoutExpired:
            out += "\n[pm3 TIMED OUT after %ss -- device busy/absent?]\n" % timeout
        except Exception as e:
            out += "\n[pm3 ERROR running '%s': %s]\n" % (joined, e)
    return out


def pm3_probe(pm3_bin, split):
    out = pm3_exec(pm3_bin, ["hw version"], split, timeout=25)
    return pm3_failure_reason(out), out


def clean(s):
    return _ANSI.sub("", s or "").replace("\r", "")


# =============================================================== parsers
def parse_info(text):
    """Pull identity fields out of `hf 15 info`. Missing fields -> None."""
    t = clean(text)
    d = {"uid": None, "type": None, "ic_ref": None, "dsfid": None, "afi": None,
         "block_count": None, "block_size": None, "no_tag": ("no tag found" in t.lower())}

    m = re.search(r"UID\.*\s*([0-9A-Fa-f]{2}(?:[ ]?[0-9A-Fa-f]{2}){3,})", t)
    if m:
        d["uid"] = re.sub(r"\s+", " ", m.group(1).strip().upper())
    m = re.search(r"TYPE(?:\.*| MATCH)\s*(.+)", t)
    if m:
        d["type"] = m.group(1).strip()
    m = re.search(r"IC ref\.*\s*0x([0-9A-Fa-f]{2})", t)
    if m:
        d["ic_ref"] = int(m.group(1), 16)
    m = re.search(r"DSFID\.*\s*0x([0-9A-Fa-f]{2})", t)
    if m:
        d["dsfid"] = int(m.group(1), 16)
    m = re.search(r"AFI\.*\s*0x([0-9A-Fa-f]{2})", t)
    if m:
        d["afi"] = int(m.group(1), 16)
    m = re.search(r"x\s+(\d+)\s+blocks", t)
    if m:
        d["block_count"] = int(m.group(1))
    m = re.search(r"(\d+)\s*\(\s*or\s*\d+\s*\)\s*bytes/blocks", t)
    if m:
        d["block_size"] = int(m.group(1))
    return d


_BLOCK_ROW = re.compile(r"([0-9A-Fa-f]{2}(?:\s[0-9A-Fa-f]{2}){3})\s*\|\s*(\d)\s*\|")
# `hf 15 dump` rows look like:  " 63/0x3F | 00 00 00 00 | 0 | ...."
_DUMP_ROW = re.compile(
    r"(?m)^\s*(?:\[[=+!\-]\]\s*)?(\d+)(?:/0x[0-9A-Fa-f]+)?\s*\|\s*"
    r"((?:[0-9A-Fa-f]{2}\s+)+[0-9A-Fa-f]{2})\s*\|")


def parse_rdbl(text):
    """Return (ok, data_hex, locked) from `hf 15 rdbl`. ok=False when the block didn't read."""
    m = _BLOCK_ROW.search(clean(text))
    if not m:
        return False, None, None
    return True, re.sub(r"\s+", " ", m.group(1).upper()), (m.group(2) == "1")


def parse_dump(text):
    """Parse `hf 15 dump` -> {block_num: 'AA BB CC DD'} for every row that read."""
    out = {}
    for m in _DUMP_ROW.finditer(clean(text)):
        out[int(m.group(1))] = re.sub(r"\s+", " ", m.group(2).strip().upper())
    return out


def block_is_zero(data_hex):
    return data_hex is not None and set(data_hex.replace(" ", "")) <= {"0"}


# =============================================================== proxmark ISO15693 ops
def pm15_info(pm3, split):
    raw = pm3_exec(pm3, ["hf 15 info"], split, timeout=40)
    return parse_info(raw), raw


def pm15_rdbl(pm3, block, split, tries=3):
    """Read one block, retrying: marginal coupling drops reads at random, so a block that reads even
    once is real. Returns (ok, data, locked, raw)."""
    raw_all = ""
    for _ in range(max(1, tries)):
        raw = pm3_exec(pm3, ["hf 15 rdbl -* -b %d" % block], split, timeout=30)
        raw_all += raw + "\n"
        ok, data, locked = parse_rdbl(raw)
        if ok:
            return True, data, locked, raw_all
    return False, None, None, raw_all


def pm15_dump(pm3, split, tries=3):
    """Read the whole card in ONE field session per attempt (far more reliable than per-block rdbl),
    merged across `tries` attempts: a block that reads in ANY attempt is real. Returns (blocks, raw)."""
    merged, raw_all = {}, ""
    for _ in range(max(1, tries)):
        raw = pm3_exec(pm3, ["hf 15 dump"], split, timeout=90)
        raw_all += raw + "\n---- dump attempt ----\n"
        for b, v in parse_dump(raw).items():
            merged.setdefault(b, v)  # keep the first successful read of each block
    return merged, raw_all


def pm15_info_retry(pm3, split, tries=3):
    info, raw_all = {"uid": None}, ""
    for _ in range(max(1, tries)):
        info, raw = pm15_info(pm3, split)
        raw_all += raw + "\n"
        if info.get("uid"):
            return info, raw_all
    return info, raw_all


def pm15_wrbl(pm3, block, data_hex, split):
    d = data_hex.replace(" ", "")
    raw = pm3_exec(pm3, ["hf 15 wrbl -* -b %d -d %s" % (block, d)], split, timeout=30)
    ok = "( ok )" in clean(raw).lower() or " ok)" in clean(raw).lower()
    fail = "( fail )" in clean(raw).lower() or "no tag found" in clean(raw).lower()
    if ok == fail:  # ambiguous -> fall back to no explicit fail marker
        ok = not fail
    return ok, raw


def pm15_csetuid(pm3, uid, gen, split):
    flag = {"gen1": "", "gen2": "--v2", "v3": "--v3"}[gen]
    raw = pm3_exec(pm3, [("hf 15 csetuid -u %s %s" % (uid, flag)).strip()], split, timeout=40)
    return ("( ok )" in clean(raw).lower()), raw


def pm15_cfg_raw(pm3, maxblock, blocksize, icref, split):
    """Send only the gen2 CFG frame (magic write 0xE0 09 47 <max> <size> <icref> 00), CRC appended."""
    frame = "02E00947%02X%02X%02X00" % (maxblock & 0xFF, blocksize & 0xFF, icref & 0xFF)
    raw = pm3_exec(pm3, ["hf 15 raw -c -w -d %s" % frame], split, timeout=30)
    return raw


# =============================================================== probes
def probe_info(ctx):
    info, raw = pm15_info(ctx["pm3"], ctx["split"])
    ctx["save_raw"]("info", raw)
    if info["no_tag"] or info["uid"] is None:
        print(C("err", "   no tag / could not read info -- is the card on the antenna?"))
        return {"ok": False, "info": info}
    print("   UID........ %s" % info["uid"])
    print("   TYPE....... %s" % (info["type"] or "?"))
    print("   IC ref..... %s   DSFID %s   AFI %s"
          % (_hx(info["ic_ref"]), _hx(info["dsfid"]), _hx(info["afi"])))
    print("   geometry... %s blocks x %s bytes  (reported)"
          % (info["block_count"], info["block_size"]))
    ctx["state"]["info"] = info
    return {"ok": True, "info": info}


def probe_capacity(ctx):
    info = ctx["state"].get("info") or pm15_info_retry(ctx["pm3"], ctx["split"])[0]
    reported = info.get("block_count") or 0
    tries = ctx["read_tries"]

    # `hf 15 dump` ZERO-FILLS blocks it can't read (like the Flipper), so it can't find the phantom
    # boundary. A phantom block instead hard-FAILS every single `rdbl`, while a real block reads
    # within a few retries. Memory is contiguous from block 0, so binary-search the highest readable
    # block with retried reads -- robust to the per-read coupling flakiness, and only ~log2(N) probes.
    if not pm15_rdbl(ctx["pm3"], 0, ctx["split"], tries=tries)[0]:
        print(C("err", "   block 0 unreadable even with %d retries -- coupling too marginal to probe;"
                       " reseat the card on the antenna." % tries))
        return {"ok": False, "skipped": "block 0 unreadable (coupling)"}

    lo, hi, last_ok, probes = 0, ((reported + 2) if reported else 255), 0, 0
    while lo <= hi:
        mid = (lo + hi) // 2
        ok, _, _, raw = pm15_rdbl(ctx["pm3"], mid, ctx["split"], tries=tries)
        ctx["save_raw"]("capacity_probe_b%03d" % mid, raw)
        probes += 1
        if ok:
            last_ok, lo = mid, mid + 1
        else:
            hi = mid - 1
    physical = last_ok + 1
    phantom = max(0, reported - physical) if reported else None

    # informational only: one dump for the data content (remember it zero-fills any phantom tail)
    dump, draw = pm15_dump(ctx["pm3"], ctx["split"], tries=1)
    ctx["save_raw"]("capacity_dump", draw)
    nonzero = sorted(b for b, v in dump.items() if not block_is_zero(v))

    print("   reported %s blocks; physical boundary via retried reads (%d probes) -> %d real blocks"
          % (reported or "?", probes, physical))
    if reported and phantom:
        print(C("warn", "   OVER-reports %d: blocks %d-%d exist only in Get-System-Info -- rdbl/wrbl FAIL there"
                % (phantom, physical, reported - 1)))
    elif reported and physical == reported:
        print(C("ok", "   physical matches reported (%d)" % reported))
    print("   non-zero data blocks (dump; note dump zero-fills phantom): %s" % (nonzero or "none"))
    ctx["state"]["physical_blocks"] = physical
    return {"ok": True, "reported": reported, "physical_blocks": physical, "phantom": phantom,
            "boundary_probes": probes, "nonzero_blocks": nonzero, "dump": dump}


def probe_magictype(ctx):
    res = {"v3_config_mode": None, "gen1_write": None, "gen2_write": None, "magic_method": None}
    # 1) V3 config-mode signature (non-destructive read of 0x14 / 0x15)
    ok_a, da, _, ra = pm15_rdbl(ctx["pm3"], 0x14, ctx["split"])
    ok_b, db, _, rb = pm15_rdbl(ctx["pm3"], 0x15, ctx["split"])
    ctx["save_raw"]("magictype_v3sig", ra + "\n---\n" + rb)
    res["v3_config_mode"] = bool(ok_a and ok_b and da == V3_SIG_A and db == V3_SIG_B)
    print("   V3 config-mode signature (0x14/0x15): %s%s"
          % (C("ok", "PRESENT (un-finalized V3 card)") if res["v3_config_mode"] else C("dim", "no"),
             ("  [%s / %s]" % (da, db)) if (ok_a and ok_b) else ""))

    # 2) gen1 / gen2 UID-write test (destructive: changes the UID). We ALWAYS restore afterwards,
    #    trying both methods, because a flaky verify must never leave the card on a test UID.
    if ctx["destructive"]:
        orig_info, _ = pm15_info_retry(ctx["pm3"], ctx["split"])  # snapshot the real UID reliably
        orig = orig_info.get("uid")
        orig_compact = orig.replace(" ", "") if orig else None
        print("   original UID (to restore): %s" % (orig or C("warn", "UNKNOWN -- restore may be impossible")))
        for gen, test_uid in (("gen1", TEST_UID_GEN1), ("gen2", TEST_UID_GEN2)):
            _, sraw = pm15_csetuid(ctx["pm3"], test_uid, gen, ctx["split"])
            info2, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])  # retried read-back
            ctx["save_raw"]("magictype_%s" % gen, sraw + "\n---info---\n" + iraw)
            got = (info2.get("uid") or "").replace(" ", "").upper()
            worked = got == test_uid.upper()
            res["%s_write" % gen] = worked
            print("   %s UID-write: %s (set %s, read %s)"
                  % (gen, C("ok", "WORKS") if worked else C("dim", "no"), test_uid, got or "?"))
            if worked and res["magic_method"] is None:
                res["magic_method"] = gen
        # ALWAYS restore -- try every method until the UID reads back as the original.
        if orig_compact:
            restored = False
            for gen in ("gen2", "gen1"):
                _, rraw = pm15_csetuid(ctx["pm3"], orig_compact, gen, ctx["split"])
                chk, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
                ctx["save_raw"]("magictype_restore_%s" % gen, rraw + "\n---info---\n" + iraw)
                if (chk.get("uid") or "").replace(" ", "").upper() == orig_compact.upper():
                    restored = True
                    break
            res["uid_restored"] = restored
            print("   restore original UID %s: %s"
                  % (orig, C("ok", "ok") if restored else
                     C("err", "FAILED -- re-clone this card from its .nfc to fix the UID")))
        else:
            res["uid_restored"] = False
            print(C("err", "   could not snapshot the original UID -- cannot restore; re-clone the card."))
    else:
        print(C("dim", "   (gen1/gen2 write test skipped -- pass --destructive to try them)"))
    ctx["state"]["magic_method"] = res["magic_method"]
    return {"ok": True, **res}


def probe_edgepages(ctx):
    if not ctx["destructive"]:
        print(C("warn", "   edgepages needs --destructive (it writes to the card). Skipping."))
        return {"ok": False, "skipped": "needs --destructive"}
    phys = ctx["state"].get("physical_blocks")
    if not phys:
        cap = probe_capacity(ctx)
        phys = cap["physical_blocks"]
    last_real, first_phantom = phys - 1, phys
    test = "AA BB CC DD"
    res = {"last_real": last_real, "first_phantom": first_phantom}

    # Snapshot block 0 (aliasing check) and the last real block (to restore). Retried reads.
    _, blk0_before, _, _ = pm15_rdbl(ctx["pm3"], 0, ctx["split"])
    ok_lr_read, lr_before, _, _ = pm15_rdbl(ctx["pm3"], last_real, ctx["split"])
    if not ok_lr_read:
        print(C("warn", "   couldn't snapshot block %d -- will not be able to restore it; skipping edgepages."
                % last_real))
        return {"ok": False, "skipped": "could not snapshot last real block (flaky coupling)"}

    # verdict is by READ-BACK, not the write command's marker (which is unreliable at marginal coupling)
    print("   writing %s to last real block %d..." % (test, last_real))
    _, wraw = pm15_wrbl(ctx["pm3"], last_real, test, ctx["split"])
    r_ok, r_data, _, _ = pm15_rdbl(ctx["pm3"], last_real, ctx["split"])
    res["last_real_readback"] = r_data
    res["last_real_real"] = bool(r_ok and r_data == test)
    print("     -> reads back %s -> %s"
          % (r_data, C("ok", "REAL, writable") if res["last_real_real"] else C("warn", "did NOT take")))

    print("   writing %s to first phantom block %d (expect failure)..." % (test, first_phantom))
    _, pwraw = pm15_wrbl(ctx["pm3"], first_phantom, test, ctx["split"])
    pr_ok, pr_data, _, _ = pm15_rdbl(ctx["pm3"], first_phantom, ctx["split"])
    _, blk0_after, _, _ = pm15_rdbl(ctx["pm3"], 0, ctx["split"])
    aliased = (blk0_before is not None and blk0_after is not None and blk0_before != blk0_after)
    res.update({"phantom_readable": pr_ok, "phantom_readback": pr_data,
                "block0_before": blk0_before, "block0_after": blk0_after, "aliased": aliased})
    print("     -> phantom block %s%s"
          % (C("warn", "READ BACK %s (real, not phantom!)" % pr_data) if (pr_ok and pr_data == test)
             else C("ok", "did not take (phantom, as expected)"),
             C("err", "   ALIASED onto block 0!") if aliased else ""))
    ctx["save_raw"]("edgepages", "last_real write:\n%s\nphantom write:\n%s" % (wraw, pwraw))

    # ALWAYS restore the last real block, verify by read-back.
    for _ in range(3):
        pm15_wrbl(ctx["pm3"], last_real, lr_before, ctx["split"])
        rok, rdata, _, _ = pm15_rdbl(ctx["pm3"], last_real, ctx["split"])
        if rok and rdata == lr_before:
            print("   restored block %d to %s: %s" % (last_real, lr_before, C("ok", "ok")))
            res["restored"] = True
            break
    else:
        print(C("err", "   restore of block %d FAILED -- re-clone the card from its .nfc." % last_real))
        res["restored"] = False
    return {"ok": True, **res}


def probe_impersonate(ctx):
    if not ctx["destructive"]:
        print(C("warn", "   impersonate needs --destructive (it rewrites the config). Skipping."))
        return {"ok": False, "skipped": "needs --destructive"}
    orig, _ = pm15_info_retry(ctx["pm3"], ctx["split"])
    orig_bc, orig_bs, orig_ic = orig.get("block_count"), orig.get("block_size"), orig.get("ic_ref")
    print("   original geometry: %s blocks x %s bytes, IC ref %s"
          % (orig_bc, orig_bs, _hx(orig_ic)))
    print(C("dim", "   NOTE: this sends a STANDALONE gen2 CFG frame; many cards only accept geometry as"))
    print(C("dim", "         part of the full UID-write sequence. For real impersonation testing, clone a"))
    print(C("dim", "         synthetic .nfc (make_test_iso15693_nfc.py) with the nfc_magic app instead."))
    tried = []
    for name, blocks, bsize, icref in ctx["impersonate_targets"]:
        raw = pm15_cfg_raw(ctx["pm3"], blocks - 1, bsize - 1, icref, ctx["split"])
        info2, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
        ctx["save_raw"]("impersonate_%s" % name, raw + "\n---info---\n" + iraw)
        got_bc, got_ic = info2.get("block_count"), info2.get("ic_ref")
        accepted = (got_bc == blocks and got_ic == icref)
        clamped = (got_bc is not None and got_bc != blocks)
        verdict = C("ok", "ACCEPTED") if accepted else \
            (C("warn", "CLAMPED to %s blocks" % got_bc) if clamped else C("err", "no change / rejected"))
        print("   -> %-14s want %d blk / IC %s : got %s blk / IC %s -> %s"
              % (name, blocks, _hx(icref), got_bc, _hx(got_ic), verdict))
        tried.append({"name": name, "want_blocks": blocks, "want_ic": icref,
                      "got_blocks": got_bc, "got_ic": got_ic, "accepted": accepted, "clamped": clamped})
    # restore original geometry
    if orig_bc and orig_bs and orig_ic is not None:
        pm15_cfg_raw(ctx["pm3"], orig_bc - 1, orig_bs - 1, orig_ic, ctx["split"])
        info3, _ = pm15_info(ctx["pm3"], ctx["split"])
        print("   restored geometry -> %s blocks, IC ref %s"
              % (info3.get("block_count"), _hx(info3.get("ic_ref"))))
    return {"ok": True, "targets": tried}


PROBES = {
    "info": (probe_info, False, "identity: UID / chip TYPE / IC ref / DSFID / AFI / reported geometry"),
    "capacity": (probe_capacity, False, "physical block count vs reported (finds the phantom tail)"),
    "magictype": (probe_magictype, False, "V3 signature; gen1/gen2 UID-write test (write part needs --destructive)"),
    "edgepages": (probe_edgepages, True, "write/read the last-real & first-phantom block; aliasing check"),
    "impersonate": (probe_impersonate, True, "make the card report other geometries / IC refs (accept/clamp)"),
}


# =============================================================== helpers
def _hx(v):
    return "0x%02X" % v if isinstance(v, int) else "?"


def ask(msg):
    try:
        input(msg)
    except EOFError:
        pass


def slug(s):
    return re.sub(r"[^0-9A-Za-z._-]+", "-", s).strip("-")[:48]


def git_commit():
    try:
        return subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=HERE,
                              capture_output=True, text=True, timeout=5).stdout.strip() or "?"
    except Exception:
        return "?"


# =============================================================== main
def main():
    ap = argparse.ArgumentParser(description="ISO15693 magic-card characterization (guided).",
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--card", help="comma-list of physical-card labels to step through (e.g. 'blank1,64blk').")
    ap.add_argument("--probes", default="info,capacity,magictype",
                    help="comma-list of probes (see --list-probes).")
    ap.add_argument("--destructive", action="store_true",
                    help="allow probes that WRITE to the card (magictype UID test, edgepages, impersonate). "
                         "They snapshot + best-effort restore, but use a blank card first.")
    ap.add_argument("--read-tries", type=int, default=6,
                    help="retries per single-block read when probing the physical boundary (a real block "
                         "reads within retries; a phantom hard-fails every time). Higher = more robust to "
                         "flaky coupling (default 6).")
    ap.add_argument("--flipper-note", action="store_true",
                    help="after the proxmark probes, prompt you to read the card in the Flipper NFC app and "
                         "note what IT reports (captures proxmark-vs-Flipper differences).")
    ap.add_argument("--pm3", default=os.environ.get("PM3", "pm3"), help="Proxmark client command (default: pm3).")
    ap.add_argument("--pm3-split", action="store_true", help="run pm3 commands one-per-invocation.")
    ap.add_argument("--out-dir", help="campaign output dir (default: tools/campaigns/iso15_<stamp>/).")
    ap.add_argument("--note", default="", help="free-text note recorded in the manifest.")
    ap.add_argument("--dry-run", action="store_true", help="print the plan + exact commands, touch nothing.")
    ap.add_argument("--no-color", action="store_true")
    ap.add_argument("--list-probes", action="store_true")
    args = ap.parse_args()

    C.enabled = (sys.stdout.isatty() and not args.no_color and os.environ.get("NO_COLOR") is None)

    if args.list_probes:
        print("Probes (name : destructive? : what it does):")
        for k, (_, dtv, desc) in PROBES.items():
            print("  %-12s %-4s %s" % (k, "WR" if dtv else "safe", desc))
        return
    if not args.card:
        sys.exit("ERROR: --card is required (a label per physical card). See --help / --list-probes.")

    cards = [c.strip() for c in args.card.split(",") if c.strip()]
    probes = [p.strip() for p in args.probes.split(",") if p.strip()]
    for p in probes:
        if p not in PROBES:
            sys.exit("ERROR: unknown probe '%s'. Known: %s (or --list-probes)." % (p, ", ".join(PROBES)))
    needs_wr = [p for p in probes if PROBES[p][1]]
    if needs_wr and not args.destructive and not args.dry_run:
        print(C("warn", "note: %s will be SKIPPED without --destructive." % ", ".join(needs_wr)))

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out_dir = args.out_dir or os.path.join(HERE, "campaigns", "iso15_%s" % stamp)

    print("=== ISO15693 magic-card characterization ===")
    print("cards      : %s" % ", ".join(cards))
    print("probes     : %s%s" % (", ".join(probes), "   [--destructive]" if args.destructive else ""))
    print("output     : %s" % out_dir)
    if args.dry_run:
        print(C("warn", "*** DRY RUN -- nothing sent to the Proxmark ***"))

    if not args.dry_run:
        if not shutil.which(shlex.split(args.pm3)[0]):
            sys.exit(C("err", "ERROR: Proxmark client '%s' not found." % args.pm3))
        reason, _ = pm3_probe(args.pm3, args.pm3_split)
        if reason:
            sys.exit(C("err", "ERROR: Proxmark3 not usable: %s." % reason) +
                     "\nClose any other pm3 session, or check the cable.")
        print(C("ok", "Proxmark3: connected."))
        os.makedirs(os.path.join(out_dir, "raw"), exist_ok=True)

    manifest = {"campaign": stamp, "note": args.note, "fw_commit": git_commit(),
                "pm3": args.pm3, "probes": probes, "destructive": bool(args.destructive), "cards": []}
    log = None if args.dry_run else open(os.path.join(out_dir, "campaign.log"), "w")

    def wlog(s=""):
        if log:
            log.write(s + "\n"); log.flush()

    def save_manifest():
        if not args.dry_run:
            with open(os.path.join(out_dir, "manifest.json"), "w") as f:
                json.dump(manifest, f, indent=2)

    wlog("# ISO15693 magic characterization %s  fw=%s  pm3=%s  destructive=%s"
         % (stamp, manifest["fw_commit"], args.pm3, args.destructive))
    wlog("# cards=%s  probes=%s  note=%s" % (cards, probes, args.note))

    try:
        for card in cards:
            crec = {"card": card, "results": {}, "flipper": None}
            hdr = "CARD '%s'   probes: %s" % (card, ", ".join(probes))
            print("\n" + C("head", "=" * 78 + "\n  " + hdr + "\n" + "=" * 78))
            wlog("\n" + "#" * 78 + "\n# " + hdr + "\n" + "#" * 78)
            if not args.dry_run:
                ask(C("pm3", "  [PM3] place card '%s' on the Proxmark3 antenna, then Enter (Ctrl-C aborts)... " % card))

            state = {}

            def save_raw(tag, text, _card=card):
                if args.dry_run:
                    return
                fn = os.path.join(out_dir, "raw", "%s_%s.txt" % (slug(_card), slug(tag)))
                with open(fn, "w") as f:
                    f.write("# card=%s probe=%s ts=%s fw=%s\n"
                            % (_card, tag, datetime.now().isoformat(timespec="seconds"), manifest["fw_commit"]))
                    f.write(text + "\n")

            ctx = {"pm3": args.pm3, "split": args.pm3_split, "state": state, "save_raw": save_raw,
                   "destructive": args.destructive, "read_tries": args.read_tries,
                   "impersonate_targets": IMPERSONATE_TARGETS}

            for p in probes:
                fn, dtv, _ = PROBES[p]
                print(C("probe", "\n  -- probe: %s --" % p))
                wlog("\n----- probe: %s (card %s) -----" % (p, card))
                if args.dry_run:
                    print(C("dim", "   (dry-run: would run probe '%s'%s)"
                            % (p, " [needs --destructive]" if dtv and not args.destructive else "")))
                    continue
                try:
                    r = fn(ctx)
                except Exception as e:  # a probe blowing up shouldn't kill the campaign
                    print(C("err", "   probe '%s' error: %s" % (p, e)))
                    r = {"ok": False, "error": str(e)}
                crec["results"][p] = r
                wlog(json.dumps({p: r}, default=str))
                save_manifest()

            if args.flipper_note and not args.dry_run:
                print(C("flip", "\n  [Flipper] now read card '%s' in the Flipper NFC app (or its NFC CLI)." % card))
                try:
                    bc = input("     block count the Flipper reports (blank to skip): ").strip()
                    note = input("     any other note (IC ref / anomaly): ").strip()
                except EOFError:
                    bc, note = "", ""
                crec["flipper"] = {"block_count": bc or None, "note": note or None}
                wlog("[flipper] block_count=%s note=%s" % (bc, note))

            manifest["cards"].append(crec)
            save_manifest()
    except KeyboardInterrupt:
        print("\n[aborted by user]")
        wlog("\n[aborted by user]")
    finally:
        if log:
            log.close()
        save_manifest()

    if not args.dry_run:
        print(C("ok", "\nDone. Campaign dir:") + "\n  %s" % out_dir)
        print("  campaign.log   (human-readable, self-labelled per card/probe)")
        print("  manifest.json  (structured results)")
        print("  raw/*.txt      (exact pm3 output of every command)")
        print("\nPaste campaign.log (or point me at the dir) and I'll analyse per card.")


if __name__ == "__main__":
    main()
