#!/usr/bin/env python3
"""ISO15693 magic-card characterization harness -- probe an (often unknown) magic NfcV tag with a
Proxmark3, guided, and record EVERYTHING with metadata into a timestamped campaign dir.

This is the ISO15693 sibling of the T5577 capture campaign. Where that one sweeps LF configs, this
one answers the questions you actually have about a *magic* 15693 card before/after cloning with the
nfc_magic app:

  info        (safe)        `hf 15 info`  -> UID, chip TYPE, IC ref, DSFID, AFI, reported block geometry
  baseline    (safe)        the PRISTINE snapshot: every block read with retries, plus a ready-to-paste
                            `hf 15 wrbl` restore script and the CFG frame that puts the advertised
                            geometry back. Run this BEFORE anything destructive -- a gen1 UID attempt
                            overwrites four blocks and cannot be undone without this record.
  capacity    (safe)        read blocks upward until a read FAILS -> the PHYSICAL block count, and how
                            many the card OVER-reports (the "fake-flash" phantom tail). Reads only.
  magictype   (safe + opt)  V3 config-mode signature read (blocks 0x14/0x15); with --destructive also
                            tries gen2 then gen1 `csetuid` to see which UID-write the card accepts, then
                            restores the original UID. GEN2 FIRST, stopping on success: gen2 sends custom
                            0xE0 frames a non-magic tag simply refuses, while gen1 sends ordinary WRITE
                            BLOCKs that ANY writable tag accepts -- so gen1-first destroys blocks
                            56/57/62/63 on every card that turns out not to be gen1.
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

It also folds each card into a STANDING INVENTORY -- `tools/tag-inventory.json` plus a rendered
`.notes/tag-inventory.md`. A campaign says what happened in one run; the inventory says what a given
physical tag IS, across runs, which is what a validation claim has to cite. The `original` section of
each entry is WRITE-ONCE: once a tag's pre-write state is recorded, no later run may replace it, because
by then this tool's own probes have written to the card.

Examples:
    # full safe characterization of one card -- no writes at all, and enough to rule gen3 in or out:
    python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes info,baseline,capacity,magictype

    # then classify gen1 vs gen2 vs non-magic, which needs a UID write (gen2 tried first):
    python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes magictype --destructive

    # re-render the inventory table from the JSON, touching no hardware:
    python3 tools/iso15693_magic_probe.py --render-inventory

    # deep test on a BLANK magic card (writes!):
    python3 tools/iso15693_magic_probe.py --card blank1 --probes info,capacity,magictype,edgepages,impersonate --destructive

    # sweep several cards, safe probes, note the Flipper's reading of each for the proxmark-vs-Flipper diff:
    python3 tools/iso15693_magic_probe.py --card "card-a,card-b,card-c" --probes info,capacity --flipper-note

    python3 tools/iso15693_magic_probe.py --list-probes
    python3 tools/iso15693_magic_probe.py --dry-run --card x --probes info,capacity,magictype,edgepages,impersonate --destructive

Requires: Proxmark3 `pm3` client on PATH (or PM3=/path). No pip needed.
"""
import argparse
import glob
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
         "block_count": None, "block_size": None, "sysinfo": None, "mfg_byte": None,
         "no_tag": ("no tag found" in t.lower())}

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
    # The raw GET SYSTEM INFO response, kept verbatim: it is the authoritative record of what the card
    # actually answered, and every parsed field above is a lossy reading of it.
    m = re.search(r"SYSINFO\.*\s*((?:[0-9A-Fa-f]{2}[ ]?)+)", t)
    if m:
        d["sysinfo"] = re.sub(r"\s+", " ", m.group(1).strip().upper())
    # uid[1] is the ISO/IEC 7816-6 manufacturer code. Recorded as the byte rather than decoded here:
    # proxmark's TYPE line already names the vendor, and a second copy of that table would be a second
    # copy to keep correct.
    if d["uid"]:
        parts = d["uid"].split()
        if len(parts) >= 2:
            try:
                d["mfg_byte"] = int(parts[1], 16)
            except ValueError:
                pass
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


def remember_geometry(ctx, info=None):
    """Cache the card's reported geometry the FIRST time we see it -- i.e. before any destructive
    probe rewrites it. `magictype`'s gen2 `csetuid` rewrites the CFG block (to proxmark's default
    geometry) as a side effect, so probes that run later must compare against this run-start snapshot,
    not a fresh (possibly already-corrupted) read. Returns {block_count, block_size, ic_ref}."""
    g = ctx["state"].get("orig_geometry")
    if g is None:
        if info is None or info.get("block_count") is None:
            info, _ = pm15_info_retry(ctx["pm3"], ctx["split"])
        g = {"block_count": info.get("block_count"),
             "block_size": info.get("block_size"),
             "ic_ref": info.get("ic_ref")}
        ctx["state"]["orig_geometry"] = g
    return g


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
    remember_geometry(ctx, info)  # earliest read wins as the run-start geometry snapshot
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

    # Informational only: one dump for the data content. TWO things make this weaker evidence than it
    # looks, and both bit on 2026-08-24 -- `hf 15 dump` zero-fills any block it cannot read, and the
    # whole command can fail outright, which on white-tag-1 produced an empty dump reported as
    # "non-zero: none". An empty result is the ABSENCE of a reading, not a reading of zeros, so it is
    # reported as unknown. The `baseline` probe is the authority here: it reads per block with retries.
    dump, draw = pm15_dump(ctx["pm3"], ctx["split"], tries=1)
    ctx["save_raw"]("capacity_dump", draw)
    nonzero = sorted(b for b, v in dump.items() if not block_is_zero(v))
    dump_read = len(dump)
    dump_ok = dump_read > 0

    print("   reported %s blocks; physical boundary via retried reads (%d probes) -> %d real blocks"
          % (reported or "?", probes, physical))
    if reported and phantom:
        print(C("warn", "   OVER-reports %d: blocks %d-%d exist only in Get-System-Info -- rdbl/wrbl FAIL there"
                % (phantom, physical, reported - 1)))
    elif reported and physical == reported:
        print(C("ok", "   physical matches reported (%d)" % reported))
    base = ctx["state"].get("baseline")
    if not dump_ok:
        print(C("warn", "   data content: dump FAILED -- nothing is known about which blocks hold data"
                        "%s" % (", but the baseline probe read them: %s"
                                % (base.get("nonzero_blocks") or "none") if base else "")))
    elif dump_read < physical:
        print(C("warn", "   non-zero data blocks: %s  (dump read only %d of %d blocks; the rest are"
                        " UNKNOWN, not zero)" % (nonzero or "none", dump_read, physical)))
    else:
        print("   non-zero data blocks (dump; note dump zero-fills phantom): %s" % (nonzero or "none"))
    ctx["state"]["physical_blocks"] = physical
    return {"ok": True, "reported": reported, "physical_blocks": physical, "phantom": phantom,
            "boundary_probes": probes, "dump": dump, "dump_ok": dump_ok, "dump_blocks_read": dump_read,
            # Only claim this when the dump actually read the whole card. inventory_update prefers the
            # baseline probe's figure anyway; this keeps the fallback from asserting a blank card.
            "nonzero_blocks": (nonzero if (dump_ok and dump_read >= physical) else None)}


def probe_baseline(ctx):
    """Capture the card's PRISTINE state, and the commands to put it back. Reads only.

    Run this before any destructive probe. The reason it exists separately from `capacity` -- which also
    dumps -- is that `capacity`'s dump is informational and single-attempt, while this one is the undo
    record: retried per block so a marginal read is not silently recorded as zeros, and emitted as a
    ready-to-paste `hf 15 wrbl` script rather than a table a human has to retype under pressure.

    What it cannot promise: a locked block cannot be restored at all, so those are called out here rather
    than discovered during a failed restore. `hf 15 dump` zero-fills blocks it cannot read, which is why
    the per-block read below is the one that decides what is real."""
    info = ctx["state"].get("info")
    if info is None:
        info, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
        ctx["save_raw"]("baseline_info", iraw)
        ctx["state"]["info"] = info
    if info.get("uid") is None:
        print(C("err", "   no tag -- cannot take a baseline."))
        return {"ok": False}
    remember_geometry(ctx, info)

    reported = info.get("block_count") or 0
    if not reported:
        print(C("err", "   card reports no block count -- nothing to bound the snapshot with."))
        return {"ok": False}

    blocks, locked, unreadable = {}, [], []
    for b in range(reported):
        ok, data, is_locked, raw = pm15_rdbl(ctx["pm3"], b, ctx["split"], tries=ctx["read_tries"])
        ctx["save_raw"]("baseline_b%03d" % b, raw)
        if ok:
            blocks["%d" % b] = data
            if is_locked:
                locked.append(b)
        else:
            unreadable.append(b)

    # The restore script. Locked blocks are emitted commented-out: a write to them will fail, and a
    # restore run that reports failures nobody expected is worse than one that says up front what it
    # cannot do.
    lines = ["# Restore card '%s' to the state recorded %s." % (ctx["card"], datetime.now().isoformat(timespec="seconds")),
             "# UID at capture: %s" % info["uid"],
             "# Paste into the pm3 client. Verify with `hf 15 info` + `hf 15 dump` afterwards.",
             "#",
             "# The UID itself is NOT restored here -- that needs `hf 15 csetuid` and the right",
             "# generation for this card. See the magictype result / the inventory entry."]
    if info.get("block_count") and info.get("block_size") and info.get("ic_ref") is not None:
        lines += ["#",
                  "# Advertised geometry, if a gen2 CFG frame moved it (max-block and size are one LESS",
                  "# than the reported counts, which is the off-by-one that makes this worth writing down):",
                  "#   hf 15 raw -c -w -d 02E00947%02X%02X%02X00"
                  % ((info["block_count"] - 1) & 0xFF, (info["block_size"] - 1) & 0xFF, info["ic_ref"] & 0xFF)]
    lines.append("")
    for b in range(reported):
        key = "%d" % b
        if key not in blocks:
            lines.append("# block %d: UNREADABLE at capture -- nothing to restore" % b)
            continue
        cmd = "hf 15 wrbl -b %d -d %s" % (b, blocks[key].replace(" ", ""))
        lines.append(("# LOCKED, write will fail: " + cmd) if b in locked else cmd)
    ctx["save_raw"]("baseline_restore_script", "\n".join(lines))

    nonzero = sorted(int(k) for k, v in blocks.items() if not block_is_zero(v))
    print("   snapshot... %d/%d blocks read%s"
          % (len(blocks), reported, (C("warn", "  (%d UNREADABLE: %s)" % (len(unreadable), unreadable))
                                     if unreadable else "")))
    if locked:
        print(C("warn", "   LOCKED blocks (cannot be restored if something writes them): %s" % locked))
    print("   non-zero at capture: %s" % (nonzero or "none -- looks blank"))
    print(C("ok", "   restore script -> raw/%s_baseline_restore_script.txt" % slug(ctx["card"])))

    snap = {"ok": True, "uid": info["uid"], "sysinfo": info.get("sysinfo"),
            "reported_blocks": reported, "block_size": info.get("block_size"),
            "ic_ref": info.get("ic_ref"), "dsfid": info.get("dsfid"), "afi": info.get("afi"),
            "type": info.get("type"), "mfg_byte": info.get("mfg_byte"),
            "blocks": blocks, "locked_blocks": locked, "unreadable_blocks": unreadable,
            "nonzero_blocks": nonzero}
    ctx["state"]["baseline"] = snap
    return snap


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

    # 2) gen2 / gen1 UID-write test (destructive: changes the UID). We ALWAYS restore afterwards,
    #    trying both methods, because a flaky verify must never leave the card on a test UID.
    #
    #    GEN2 IS TRIED FIRST, AND A SUCCESS STOPS THERE. That order is a data-safety requirement, not a
    #    preference, and it is the opposite of what this probe did until 2026-08-24:
    #
    #      gen2 (armsrc/iso15693.c:3216, SetTag15693Uid_v2) sends four CUSTOM 0xE0 frames. A tag that
    #        does not implement the magic command simply refuses them: nothing is written anywhere.
    #      gen1 (armsrc/iso15693.c:3166, SetTag15693Uid) sends four ORDINARY WRITE BLOCK frames, at
    #        blocks 0x3E, 0x3F, 0x38 and 0x39. Any writable ISO15693 tag accepts an ordinary write, so on
    #        a non-magic tag this DESTROYS blocks 56, 57, 62 and 63 -- and on a gen2 card those four are
    #        ordinary user data.
    #
    #    So gen1-first spends four data blocks on every card that is not gen1, to learn something the
    #    harmless probe would have told us. Run the `baseline` probe before this one either way: a gen1
    #    write cannot be undone without a record of what was there.
    if ctx["destructive"]:
        orig_info, _ = pm15_info_retry(ctx["pm3"], ctx["split"])  # snapshot the real UID reliably
        orig = orig_info.get("uid")
        orig_compact = orig.replace(" ", "") if orig else None
        geo_before = remember_geometry(ctx, orig_info)  # capture geometry BEFORE gen2 clobbers the CFG
        print("   original UID (to restore): %s" % (orig or C("warn", "UNKNOWN -- restore may be impossible")))
        for gen, test_uid in (("gen2", TEST_UID_GEN2), ("gen1", TEST_UID_GEN1)):
            if gen == "gen1":
                # Checked HERE rather than before gen2, because until gen2 has failed there is nothing
                # to warn about: gen2 writes no data blocks, and a success breaks out below.
                found = find_baseline(ctx["card"], ctx["state"])
                if found:
                    print(C("dim", "   gen1 undo record: %s" % found))
                else:
                    print(C("warn", "   ! NO baseline snapshot for this card. The gen1 attempt writes"
                                    " blocks 56/57/62/63 with ordinary WRITE BLOCKs, which any writable"
                                    "\n     tag accepts -- with no snapshot that is unrecoverable."))
                    ask(C("warn", "     Enter to go ahead anyway, Ctrl-C to stop and run"
                                  " --probes baseline first... "))
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
            if worked:
                # Stop at the first method that works. On a gen2 card, going on to try gen1 would write
                # the four backdoor blocks for no new information.
                print(C("dim", "   (stopping here -- no need to try the destructive method)"))
                break
        # Restore, but only what actually needs restoring, and only with the method that demonstrably
        # works on this card. The old form looped ("gen2", "gen1") unconditionally, which had two bad
        # consequences neither of which is hypothetical:
        #
        #   on a NON-MAGIC tag no write moved the UID, so there was nothing to restore -- yet it still
        #     sent a gen1 csetuid, i.e. four more ordinary WRITE BLOCKs at 56/57/62/63, and then
        #     reported "FAILED -- re-clone this card from its .nfc" about a UID that never changed.
        #   on a GEN2 card whose gen2 restore came back flaky, the gen1 fallback would write those same
        #     four blocks -- which on gen2 are ordinary user data. That is the identical hazard the
        #     classification order above exists to avoid, sitting in the recovery path.
        if orig_compact:
            chk, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
            ctx["save_raw"]("magictype_restore_check", iraw)
            now = (chk.get("uid") or "").replace(" ", "").upper()
            if now == orig_compact.upper():
                res["uid_restored"] = True
                res["restore_needed"] = False
                print("   restore original UID: %s" % C("ok", "not needed -- UID never moved"))
            elif res["magic_method"] is None:
                # The UID differs but nothing we sent could have moved it. Do NOT start writing blocks
                # on a guess; say what was seen and let a human decide.
                res["uid_restored"] = False
                res["restore_needed"] = True
                print(C("err", "   UID reads %s, expected %s, and no write method worked here."
                               " NOT attempting a blind restore -- check the card and the raw logs."
                        % (now or "?", orig)))
            else:
                gen = res["magic_method"]
                restored = False
                for attempt in range(2):
                    _, rraw = pm15_csetuid(ctx["pm3"], orig_compact, gen, ctx["split"])
                    chk, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
                    ctx["save_raw"]("magictype_restore_%s_%d" % (gen, attempt), rraw + "\n---info---\n" + iraw)
                    if (chk.get("uid") or "").replace(" ", "").upper() == orig_compact.upper():
                        restored = True
                        break
                res["uid_restored"] = restored
                res["restore_needed"] = True
                print("   restore original UID %s via %s: %s"
                      % (orig, gen, C("ok", "ok") if restored else
                         C("err", "FAILED -- re-clone this card from its .nfc. Deliberately NOT falling"
                                  " back to the other generation, which would write 56/57/62/63.")))
        else:
            res["uid_restored"] = False
            print(C("err", "   could not snapshot the original UID -- cannot restore; re-clone the card."))

        # gen2 `csetuid` writes proxmark's DEFAULT CFG block (geometry) as a side effect, and so does
        # the UID-restore write above. Detect and report that -- otherwise the card silently advertises
        # a different block-count / IC ref than it started with, and later probes read the wrong baseline.
        if geo_before.get("block_count") is not None:
            after, araw = pm15_info_retry(ctx["pm3"], ctx["split"])
            ctx["save_raw"]("magictype_geometry", araw)
            geo_after = {"block_count": after.get("block_count"),
                         "block_size": after.get("block_size"), "ic_ref": after.get("ic_ref")}
            dirty = (geo_after["block_count"] != geo_before["block_count"]
                     or geo_after["ic_ref"] != geo_before["ic_ref"])
            if dirty and all(geo_before.get(k) is not None for k in ("block_count", "block_size", "ic_ref")):
                # best-effort standalone-CFG restore (works only on cards that accept standalone CFG)
                pm15_cfg_raw(ctx["pm3"], geo_before["block_count"] - 1,
                             geo_before["block_size"] - 1, geo_before["ic_ref"], ctx["split"])
                after2, _ = pm15_info_retry(ctx["pm3"], ctx["split"])
                if (after2.get("block_count") == geo_before["block_count"]
                        and after2.get("ic_ref") == geo_before["ic_ref"]):
                    dirty = False
                    geo_after = {"block_count": after2.get("block_count"),
                                 "block_size": after2.get("block_size"), "ic_ref": after2.get("ic_ref")}
                    print(C("ok", "   geometry restored -> %s blk / IC %s"
                            % (geo_after["block_count"], _hx(geo_after["ic_ref"]))))
            res["geometry_before"] = geo_before
            res["geometry_after"] = geo_after
            res["geometry_dirty"] = dirty
            if dirty:
                print(C("err", "   ! geometry CHANGED by the gen2 test: %s blk / IC %s  (was %s blk / IC %s)"
                        % (geo_after["block_count"], _hx(geo_after["ic_ref"]),
                           geo_before["block_count"], _hx(geo_before["ic_ref"]))))
                print(C("err", "     proxmark can't restore it -- this card ignores standalone CFG writes;"))
                print(C("warn", "     re-clone this card from its .nfc via the nfc_magic app to restore its identity."))
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
    # canonical = the card's geometry at run start (before any destructive probe touched the CFG);
    # live = what it reports right now (a preceding `magictype` gen2 test may have already rewritten it).
    canon = remember_geometry(ctx)
    live, _ = pm15_info_retry(ctx["pm3"], ctx["split"])
    live_bc, live_ic = live.get("block_count"), live.get("ic_ref")
    print("   run-start geometry: %s blocks x %s bytes, IC ref %s"
          % (canon.get("block_count"), canon.get("block_size"), _hx(canon.get("ic_ref"))))
    if live_bc != canon.get("block_count") or live_ic != canon.get("ic_ref"):
        print(C("warn", "   card currently reports %s blk / IC %s -- already changed by an earlier probe "
                        "(the magictype gen2 test)." % (live_bc, _hx(live_ic))))
    print(C("dim", "   NOTE: this sends a STANDALONE gen2 CFG frame; many cards (incl. this one) only accept"))
    print(C("dim", "         geometry as part of the FULL UID-write sequence, so standalone frames are ignored."))
    print(C("dim", "         Real impersonation is tested by cloning a synthetic .nfc with the nfc_magic app"))
    print(C("dim", "         (make_test_iso15693_nfc.py -> flipper_ground_truth.py), not by this probe."))
    tried = []
    for name, blocks, bsize, icref in ctx["impersonate_targets"]:
        raw = pm15_cfg_raw(ctx["pm3"], blocks - 1, bsize - 1, icref, ctx["split"])
        info2, iraw = pm15_info_retry(ctx["pm3"], ctx["split"])
        ctx["save_raw"]("impersonate_%s" % name, raw + "\n---info---\n" + iraw)
        got_bc, got_ic = info2.get("block_count"), info2.get("ic_ref")
        accepted = (got_bc == blocks and got_ic == icref)
        unchanged = (got_bc == live_bc and got_ic == live_ic)
        if accepted:
            verdict, tag = C("ok", "ACCEPTED"), "accepted"
        elif unchanged:
            verdict, tag = C("warn", "no change (standalone CFG ignored)"), "ignored"
        else:
            verdict, tag = C("err", "unexpected -> %s blk / IC %s" % (got_bc, _hx(got_ic))), "unexpected"
        print("   -> %-14s want %d blk / IC %s : got %s blk / IC %s -> %s"
              % (name, blocks, _hx(icref), got_bc, _hx(got_ic), verdict))
        tried.append({"name": name, "want_blocks": blocks, "want_ic": icref, "got_blocks": got_bc,
                      "got_ic": got_ic, "accepted": accepted, "result": tag})
    # restore to how the card was when this probe STARTED (leave-as-found); does nothing on cards that
    # ignore standalone CFG -- honestly report whatever the card ends up reporting.
    if live_bc and live.get("block_size") and live_ic is not None:
        pm15_cfg_raw(ctx["pm3"], live_bc - 1, live.get("block_size") - 1, live_ic, ctx["split"])
        info3, _ = pm15_info(ctx["pm3"], ctx["split"])
        print("   card now reports -> %s blocks, IC ref %s"
              % (info3.get("block_count"), _hx(info3.get("ic_ref"))))
    return {"ok": True, "run_start": canon, "live_at_start": {"block_count": live_bc, "ic_ref": live_ic},
            "targets": tried}


def probe_writespan(ctx):
    """Does WRITE BLOCK obey the card's ADVERTISED block count, or its PHYSICAL capacity?

    The clone app currently caps data writes at the target's advertised block count. That's only the
    right thing to do if the card actually refuses writes beyond that count. This probe settles it.

    Meaningful only when the card advertises FEWER blocks than it physically has -- so there's a gap to
    probe. Clone a small source first (e.g. slix_28 -> advertises 28) onto a card that is physically
    larger (our magic target is physically 64). Then this write-tests a ladder of blocks from just
    below the advertised count up through/just past the physical boundary, snapshotting and restoring
    each, and reports the highest block that actually accepts a write:
      - highest writable > advertised  -> writes follow PHYSICAL capacity. The advertised count does
          NOT gate writes -> the app should attempt every source block and report only true failures.
      - highest writable == advertised -> the card GATES writes by the advertised count -> that count
          is the real limit and the app must report it as such.
    """
    if not ctx["destructive"]:
        print(C("warn", "   writespan needs --destructive (it writes to the card). Skipping."))
        return {"ok": False, "skipped": "needs --destructive"}

    info, _ = pm15_info_retry(ctx["pm3"], ctx["split"])
    adv = info.get("block_count")
    if not adv:
        print(C("err", "   could not read the advertised block count; aborting writespan."))
        return {"ok": False, "error": "no advertised count"}
    phys = ctx["state"].get("physical_blocks")
    if not phys:
        phys = probe_capacity(ctx).get("physical_blocks")

    # Only need a small window just past the advertised boundary to decide: a couple of blocks below
    # (known-good baseline) up to a few above. Walking to the physical top would be many slow PM3 calls.
    lo = max(0, adv - 2)
    hi = min(255, ctx.get("writespan_max") or (adv + 6))
    print("   advertised %s blocks; physical %s; probing writes to blocks %d..%d"
          % (adv, phys if phys else "?", lo, hi))
    no_gap = bool(phys) and adv >= phys
    if no_gap:
        print(C("warn", "   NOTE: advertised (%d) >= physical (%d) -- no advertised<physical gap to test."
                % (adv, phys)))
        print(C("warn", "         Clone a SMALL source (e.g. slix_28 -> 28 blocks) with the app first, re-run."))

    ladder = []
    for b in range(lo, hi + 1):
        ok_r, orig, _, _ = pm15_rdbl(ctx["pm3"], b, ctx["split"])
        marker = "5A %02X A5 %02X" % (b & 0xFF, b & 0xFF)
        pm15_wrbl(ctx["pm3"], b, marker, ctx["split"])
        ok_rb, rb, _, _ = pm15_rdbl(ctx["pm3"], b, ctx["split"])
        took = ok_rb and rb == marker
        restored = None
        if took:
            restore_to = orig if ok_r else "00 00 00 00"
            for _ in range(3):
                pm15_wrbl(ctx["pm3"], b, restore_to, ctx["split"])
                rok, rdata, _, _ = pm15_rdbl(ctx["pm3"], b, ctx["split"])
                if rok and rdata == restore_to:
                    restored = restore_to
                    break
        ladder.append({"block": b, "read_before": orig if ok_r else None,
                       "write_took": took, "restored": restored})
        rel = "<adv" if b < adv else ("=adv" if b == adv else ">adv")
        print("   block %3d (%-4s) %s%s" % (b, rel,
              C("ok", "WRITE ok") if took else C("dim", "no write"),
              "" if (restored is not None or not took) else C("err", "  [NOT restored]")))

    took_blocks = [x["block"] for x in ladder if x["write_took"]]
    highest = max(took_blocks) if took_blocks else None
    unrestored = [x["block"] for x in ladder if x["write_took"] and x["restored"] is None]
    res = {"advertised": adv, "physical": phys, "highest_writable": highest,
           "unrestored": unrestored, "ladder": ladder}
    ctx["save_raw"]("writespan", json.dumps(res, indent=2))

    if highest is None:
        res["writes_follow"] = "inconclusive"
        print(C("err", "   -> no block accepted a write; inconclusive (coupling / wrong card?)."))
    elif no_gap:
        res["writes_follow"] = "inconclusive"
        print(C("warn", "   -> highest writable %d, but advertised==physical so this can't distinguish"
                % highest))
        print(C("warn", "      advertised-gating from physical-gating. Re-run with advertised < physical."))
    elif highest >= adv:
        res["writes_follow"] = "physical"
        print(C("ok", "   -> writes SUCCEED past the advertised count (highest writable %d, advertised %d)."
                % (highest, adv)))
        print(C("ok", "      Advertised count does NOT gate writes -> app should write ALL source blocks"))
        print(C("ok", "      and report only the blocks that truly fail (the physical limit)."))
    else:
        res["writes_follow"] = "advertised"
        print(C("warn", "   -> writes STOP at the advertised count (highest writable %d < advertised %d)."
                % (highest, adv)))
        print(C("warn", "      The card gates writes by the advertised count -> THAT is the real limit."))
    if unrestored:
        print(C("err", "   ! could not restore blocks %s -- re-clone this card from its .nfc." % unrestored))
    return {"ok": True, **res}


PROBES = {
    "info": (probe_info, False, "identity: UID / chip TYPE / IC ref / DSFID / AFI / reported geometry"),
    "baseline": (probe_baseline, False, "PRISTINE snapshot + a ready-to-paste restore script (run before any write)"),
    "capacity": (probe_capacity, False, "physical block count vs reported (finds the phantom tail)"),
    "magictype": (probe_magictype, False, "V3 signature; gen1/gen2 UID-write test (write part needs --destructive)"),
    "edgepages": (probe_edgepages, True, "write/read the last-real & first-phantom block; aliasing check"),
    "impersonate": (probe_impersonate, True, "does the card accept a standalone CFG frame for another geometry?"),
    "writespan": (probe_writespan, True, "does WRITE BLOCK obey the advertised count or physical capacity?"),
}


# =============================================================== inventory
#
# A campaign dir answers "what happened in that run". The inventory answers "what IS this tag", across
# runs and across months, which is the question an upstream submission needs: which physical tag was a
# given result measured on, and what was it before anybody wrote to it.
#
# One rule makes it trustworthy: THE ORIGINAL SECTION IS WRITE-ONCE. A later run may add a
# classification, or correct a classification, but it may never restate what the tag looked like
# originally -- by then the tool's own probes have written to it, so a fresh read is not the original.
INVENTORY_JSON = os.path.join(HERE, "tag-inventory.json")
INVENTORY_MD = os.path.join(os.path.dirname(HERE), ".notes", "tag-inventory.md")


def identify(pm3, split, expect=None, path=None):
    """Read whatever tag is on the antenna and say which inventory entry it is.

    This exists because labels live on paper and UIDs live on silicon. Three of the white-tags are
    physically identical and differ only in the last two UID bytes, and they are NOT interchangeable --
    one has been write-probed and one is the untouched control -- so "which tag is this" has to be
    answerable without trusting how they were put away.

    With `expect`, it is a precondition rather than a question: non-zero exit if the tag on the antenna
    is not the one you meant to write to."""
    inv = inventory_load(path)
    if inv is None:
        return 2
    info, raw = pm15_info_retry(pm3, split)
    uid = info.get("uid")
    if not uid:
        print(C("err", "no tag on the antenna (or it would not read)."))
        return 2

    def norm(u):
        return (u or "").replace(" ", "").upper()

    match = None
    for label in sorted(inv.get("tags", {})):
        if norm((inv["tags"][label].get("original") or {}).get("uid")) == norm(uid):
            match = label
            break

    print("UID on antenna : %s" % uid)
    print("chip           : %s" % (info.get("type") or "?"))
    print("geometry       : %s blocks x %s bytes, IC ref %s"
          % (info.get("block_count"), info.get("block_size"), _hx(info.get("ic_ref"))))
    if match is None:
        print(C("warn", "inventory       : NOT RECORDED -- this tag is new, or its UID has been changed"
                        " since it was recorded (a magic card's UID is not an identity)."))
    else:
        e = inv["tags"][match]
        o, cl = e.get("original", {}), e.get("classification", {})
        print(C("ok", "inventory       : %s" % match))
        print("  verdict      : %s" % cl.get("verdict", "unclassified"))
        print("  at capture   : %s blocks advertised, data in %s"
              % (o.get("advertised_blocks"), _data_cell(o.get("nonzero_blocks"))))
        if e.get("note"):
            print("  note         : %s" % e["note"])

    if expect is None:
        return 0 if match else 1
    if match == expect:
        print(C("ok", "\nCONFIRMED: this is '%s'." % expect))
        return 0
    print(C("err", "\nWRONG TAG. Expected '%s', this is %s."
                   % (expect, "'%s'" % match if match else "not in the inventory")))
    if match is None:
        print(C("err", "Do not write to it on the assumption it is '%s'." % expect))
    return 1


def inventory_load(path=None):
    path = path or INVENTORY_JSON
    if not os.path.exists(path):
        return {"tags": {}}
    try:
        with open(path) as f:
            d = json.load(f)
        d.setdefault("tags", {})
        return d
    except Exception as e:
        print(C("warn", "note: could not read %s (%s) -- not overwriting it." % (path, e)))
        return None


def inventory_update(card, results, campaign, path=None):
    """Fold one card's probe results into the inventory. Returns (entry, what_changed) or (None, why)."""
    path = path or INVENTORY_JSON
    inv = inventory_load(path)
    if inv is None:
        return None, "inventory unreadable"

    entry = inv["tags"].setdefault(card, {})
    changed = []
    # `expected` is hand-seeded and never written here. It records what the LISTING claimed, which is a
    # fact about the listing, not about the tag -- so a probe can disagree with it and both stay true.

    base = results.get("baseline") or {}
    info = (results.get("info") or {}).get("info") or {}
    cap = results.get("capacity") or {}
    mag = results.get("magictype") or {}

    # --- original: write-once, and only from a source that was read before anything wrote
    if "original" not in entry:
        src = base if base.get("ok") else (info if info.get("uid") else None)
        if src:
            entry["original"] = {
                "uid": src.get("uid"),
                "type": src.get("type"),
                "mfg_byte": src.get("mfg_byte"),
                "sysinfo": src.get("sysinfo"),
                "advertised_blocks": src.get("reported_blocks") or src.get("block_count"),
                "block_size": src.get("block_size"),
                "ic_ref": src.get("ic_ref"),
                "dsfid": src.get("dsfid"),
                "afi": src.get("afi"),
                "locked_blocks": base.get("locked_blocks"),
                # None means NOT KNOWN, [] means read and found empty. dict.get's default only fires
                # when the key is absent, so pick explicitly: the baseline probe reads per block with
                # retries and is the authority; capacity's dump can fail and return None.
                "nonzero_blocks": (base.get("nonzero_blocks") if base.get("ok")
                                   else cap.get("nonzero_blocks")),
                "captured": datetime.now().isoformat(timespec="seconds"),
                "campaign": campaign,
                "from_baseline_probe": bool(base.get("ok")),
            }
            changed.append("original")
    elif base.get("ok") or info.get("uid"):
        # Do not touch it -- but say so, because silently ignoring a fresh read looks like a bug.
        changed.append("original kept (already recorded %s)" % entry["original"].get("captured", "?"))

    if cap.get("ok"):
        phys = {"physical_blocks": cap.get("physical_blocks"), "phantom_blocks": cap.get("phantom"),
                "measured": datetime.now().isoformat(timespec="seconds"), "campaign": campaign}
        if entry.get("physical") != phys:
            entry.setdefault("physical", phys)
            changed.append("physical")

    if mag.get("ok") is not False and mag:
        cls = entry.setdefault("classification", {})
        if mag.get("v3_config_mode") is not None:
            cls["gen3_signature"] = mag["v3_config_mode"]
        for k in ("gen1_write", "gen2_write"):
            if mag.get(k) is not None:
                cls[k] = mag[k]
        verdict = classify(cls)
        if verdict != cls.get("verdict"):
            cls["verdict"] = verdict
            cls["campaign"] = campaign
            changed.append("classification -> %s" % verdict)

    inv["updated"] = datetime.now().isoformat(timespec="seconds")
    with open(path, "w") as f:
        json.dump(inv, f, indent=2, sort_keys=True)
    inventory_render(inv)
    return entry, ", ".join(changed) if changed else "no change"


def check_expected(entry):
    """Compare the seller's claim against what was measured. Returns (verdict, [notes]).

    A mismatch is a RESULT, not an error: a tag advertising fewer blocks than the listing says is the
    fake-flash / programmed-count case this whole project exists around, and one advertising more is the
    phantom tail. So this reports the difference and never "corrects" either side."""
    exp = entry.get("expected")
    if not exp:
        return "no claim recorded", []
    o = entry.get("original", {})
    ph = entry.get("physical", {})
    cl = entry.get("classification", {})
    notes, bad = [], False

    # PHYSICAL capacity is what a listing's block count is really a claim about, so that is the one that
    # can be a mismatch. The ADVERTISED count differing is not a fault on magic silicon at all -- the
    # gen2 CFG frame programs it, so a card cloned from a smaller source under-reports and one with fake
    # flash over-reports. Scoring that as MISMATCH would flag the normal case on every magic tag and
    # teach a reader to ignore the column, so it is recorded as a note and left out of the verdict.
    adv, phys, want = o.get("advertised_blocks"), ph.get("physical_blocks"), exp.get("blocks")
    if want is not None and phys is not None and phys != want:
        notes.append("physical %d, listed %d" % (phys, want)); bad = True
    if want is not None and adv is not None and adv != want:
        if phys is None:
            notes.append("advertises %d, listed %d (physical not measured yet)" % (adv, want))
        elif phys == want:
            notes.append("advertises %d but physically %d as listed -- programmed count" % (adv, phys))
        else:
            notes.append("advertises %d, listed %d" % (adv, want))

    bs, want_bs = o.get("block_size"), exp.get("block_size")
    if want_bs is not None and bs is not None and bs != want_bs:
        notes.append("block size %d, listed %d" % (bs, want_bs)); bad = True

    verdict = cl.get("verdict")
    want_magic = exp.get("magic")
    if want_magic is not None and verdict and not verdict.startswith("unclassified"):
        found_magic = verdict.startswith(("gen1", "gen2", "gen3"))
        if want_magic and not found_magic and verdict == "no gen1/gen2/gen3":
            if exp.get("pm3_writable"):
                # The seller's claim was specific: proxmark can write this one. That claim failed.
                notes.append("marked pm3 and labelled UID-changeable, but no gen1/gen2/gen3 took")
                bad = True
            else:
                # No pm3 mark. "proxmark cannot write it" is what the seller said about these, so a
                # negative here CONFIRMS the annotation rather than contradicting the UID claim. Not a
                # mismatch -- and not a licence to call the tag plain either.
                notes.append("no gen1/gen2/gen3, and no pm3 mark -- consistent with the custom-app"
                             " magic type the seller described; UID claim neither confirmed nor refuted")
        elif want_magic and not found_magic:
            notes.append("labelled UID-changeable but classifies %s" % verdict); bad = True
        elif not want_magic and found_magic:
            notes.append("listed as plain but classifies %s" % verdict); bad = True

    # "matches listing" has to mean every claim was CHECKED, not just that nothing contradicted one.
    # A tag whose magic status was never probed matches nothing yet, and saying otherwise is the same
    # overclaim as a coverage note that counts untested cases as passing.
    untested = []
    if want is not None and phys is None:
        untested.append("physical capacity")
    if want_magic is not None and (not verdict or verdict.startswith("unclassified")):
        untested.append("magic")
    if bad:
        return "MISMATCH", notes
    if notes:
        return "differs", notes
    if untested and (adv is not None or verdict):
        return "matches so far", ["%s not tested" % " and ".join(untested)]
    if untested:
        return "not yet measured", []
    return "matches listing", []


def classify(cls):
    """Turn the probe flags into a verdict, and be explicit about what is NOT yet decidable.

    gen1 deliberately does not resolve on the write alone: an ordinary writable tag accepts the same
    four WRITE BLOCK frames, so a gen1 write that "works" only means the frames landed. What separates a
    gen1 magic tag from a tag whose blocks 56/57 just got overwritten is whether the UID MOVED, and the
    probe reads that back -- so gen1_write True already means the UID changed. A tag that took the writes
    without moving its UID shows up as gen1_write False, which is why that case reads as non-magic rather
    than unknown."""
    if cls.get("gen3_signature"):
        return "gen3 (un-finalized, config mode)"
    if cls.get("gen2_write"):
        return "gen2 magic"
    if cls.get("gen1_write"):
        return "gen1 magic"
    if cls.get("gen2_write") is False and cls.get("gen1_write") is False:
        # NOT "non-magic". All this establishes is that none of the three methods proxmark implements
        # moved the UID. The 2026-09-08 batch arrived with some tags hand-marked "pm3" by the seller and
        # others described as writable only with a custom application he did not supply -- so a magic
        # mechanism proxmark cannot reach is a live possibility, not a hypothetical. Whether this reading
        # contradicts a tag's label is a question for check_expected, which knows what was claimed;
        # the verdict's job is to report what was measured.
        return "no gen1/gen2/gen3"
    if cls.get("gen2_write") is False:
        return "not gen2; gen1 untested"
    return "unclassified (no write probe run)"


def find_baseline(card, state=None):
    """Where this card's undo record lives, if anywhere.

    ctx["state"] is per-INVOCATION, so a baseline taken in an earlier campaign is invisible to it -- which
    is exactly the recommended workflow (safe pass in one run, destructive probe in another) and made the
    first real use of this warning a false alarm. So look on disk too, and name the file: "a baseline
    exists" is worth much less to someone about to lose four blocks than the path to the restore script."""
    if state and state.get("baseline"):
        return "captured in this run"
    hits = sorted(glob.glob(os.path.join(HERE, "campaigns", "*", "raw",
                                         "%s_baseline_restore_script.txt" % slug(card))))
    if hits:
        return hits[-1]  # campaign dirs sort chronologically by name
    inv = inventory_load()
    if inv and ((inv.get("tags", {}).get(card, {}) or {}).get("original") or {}).get("from_baseline_probe"):
        return "recorded in the inventory, but the campaign file is missing"
    return None


def _data_cell(nonzero):
    """None is NOT KNOWN; [] is read-and-empty. Collapsing the two is how a failed dump gets reported
    as a blank card, which is what happened on 2026-08-24 before probe_capacity learned the difference."""
    if nonzero is None:
        return "unknown"
    return str(nonzero) if nonzero else "blank"


def inventory_render(inv, path=None):
    path = path or INVENTORY_MD
    rows = []
    for card in sorted(inv.get("tags", {})):
        e = inv["tags"][card]
        o = e.get("original", {})
        ph = e.get("physical", {})
        cl = e.get("classification", {})
        adv = o.get("advertised_blocks")
        phys = ph.get("physical_blocks")
        geo = "%s adv" % (adv if adv is not None else "?")
        if phys is not None:
            geo += " / %d phys" % phys
            if adv is not None and phys != adv:
                geo += " (%+d)" % (phys - adv)
        exp_verdict, exp_notes = check_expected(e)
        exp_cell = exp_verdict if not exp_notes else "%s — %s" % (exp_verdict, "; ".join(exp_notes))
        rows.append((card, cl.get("verdict", "unclassified"), o.get("uid") or "?",
                     (o.get("type") or "?"), geo,
                     "%s / %s / %s" % (_hx(o.get("ic_ref")), _hx(o.get("dsfid")), _hx(o.get("afi"))),
                     _data_cell(o.get("nonzero_blocks")),
                     "yes" if o.get("from_baseline_probe") else "no",
                     exp_cell))
    try:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as f:
            f.write("# ISO15693 tag inventory\n\n")
            f.write("Generated by `tools/iso15693_magic_probe.py`; source of truth is\n")
            f.write("`tools/tag-inventory.json`. Do not hand-edit this file -- it is rewritten on every run.\n\n")
            f.write("The **original** columns are write-once, recorded before any probe wrote to the tag.\n")
            f.write("A blank restore column means the snapshot came from `info`/`capacity` rather than the\n")
            f.write("`baseline` probe, so there is no per-block record to restore from.\n\n")
            f.write("**vs listing** compares what the seller claimed against what was measured. A mismatch is a\n")
            f.write("result, not an error -- an advertised count below the listing is the programmed-count case and\n")
            f.write("one above it is the phantom tail. Neither side gets corrected.\n\n")
            f.write("| tag | verdict | original UID | proxmark TYPE | blocks | IC ref / DSFID / AFI | data at capture | restorable | vs listing |\n")
            f.write("|---|---|---|---|---|---|---|---|---|\n")
            for r in rows:
                f.write("| " + " | ".join("`%s`" % r[2] if i == 2 else str(r[i]) for i in range(len(r))) + " |\n")
            if not rows:
                f.write("| _(none recorded yet)_ | | | | | | | | |\n")
            f.write("\nUpdated %s\n" % inv.get("updated", "?"))
    except Exception as e:
        print(C("warn", "note: could not write %s (%s)." % (path, e)))


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
    ap.add_argument("--writespan-max", type=int, default=None,
                    help="highest block index the writespan probe tries (default: max(advertised,physical)+4).")
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
    ap.add_argument("--no-inventory", action="store_true",
                    help="do not fold results into tools/tag-inventory.json / .notes/tag-inventory.md.")
    ap.add_argument("--inventory", default=None,
                    help="path to the inventory JSON (default: tools/tag-inventory.json).")
    ap.add_argument("--render-inventory", action="store_true",
                    help="rewrite .notes/tag-inventory.md from the JSON and exit; touches no hardware.")
    ap.add_argument("--identify", action="store_true",
                    help="read the tag on the antenna and say which inventory entry it is. Read-only. "
                         "With --card <label>, ASSERTS it is that tag and exits non-zero if not.")
    args = ap.parse_args()

    C.enabled = (sys.stdout.isatty() and not args.no_color and os.environ.get("NO_COLOR") is None)

    if args.render_inventory:
        inv = inventory_load(args.inventory)
        if inv is None:
            sys.exit(1)
        inventory_render(inv)
        print("rendered %s (%d tag%s)"
              % (INVENTORY_MD, len(inv.get("tags", {})), "" if len(inv.get("tags", {})) == 1 else "s"))
        return

    if args.identify:
        if not shutil.which(shlex.split(args.pm3)[0]):
            sys.exit(C("err", "ERROR: Proxmark client '%s' not found." % args.pm3))
        reason, _ = pm3_probe(args.pm3, args.pm3_split)
        if reason:
            sys.exit(C("err", "ERROR: Proxmark3 not usable: %s." % reason))
        cards = [c.strip() for c in (args.card or "").split(",") if c.strip()]
        if len(cards) > 1:
            sys.exit("ERROR: --identify checks ONE tag; pass a single --card label, or none.")
        sys.exit(identify(args.pm3, args.pm3_split, cards[0] if cards else None, args.inventory))

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
                   "writespan_max": args.writespan_max, "card": card,
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

            if not args.no_inventory and not args.dry_run:
                entry, what = inventory_update(card, crec["results"], stamp, args.inventory)
                if entry is None:
                    print(C("warn", "\n  inventory: skipped (%s)" % what))
                else:
                    v = (entry.get("classification") or {}).get("verdict", "unclassified")
                    print(C("ok", "\n  inventory: '%s' -> %s   [%s]" % (card, v, what)))
                    crec["inventory"] = {"verdict": v, "changed": what}

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
