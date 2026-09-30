#!/usr/bin/env python3
"""Apply fold rules to every dev commit from each rule's owner to HEAD.

  engine.py dry     report, per rule, where it applied and any commit where it did not
  engine.py tip     print the tip diff the rules produce
  engine.py map     write blobs and the index-filter map (orig-sha mode blob path)

A rule is either
  sub:  exact `old` -> `new`, which must occur exactly once
  para: the comment paragraph holding `anchor`; each (old, new) in `subs` applied to its prose
        (exactly once each), then rewrapped at the file's width. `from_anchor` stops the paragraph
        extending above the anchor line; `stop_before` ends it before the line holding that text.
Every rule must apply at EVERY commit from its owner through HEAD. A miss is an error, not a skip.
"""
import difflib
import os
import re
import subprocess
import sys
import textwrap
from pathlib import Path

REPO = str(Path(__file__).resolve().parents[2])  # tools/fold/ -> the dev repo
# The rules module is read from, and the index map written to, FOLD_WORK -- a scratch directory,
# never the repo. RULES names the module (rules9 -> $FOLD_WORK/rules9.py).
HERE = Path(os.environ["FOLD_WORK"]).resolve()
sys.path.insert(0, str(HERE))
import importlib  # noqa: E402
_mod = importlib.import_module(os.environ.get("RULES", "rules1"))
RULES, WIDTH, RANGE_BASE = _mod.RULES, _mod.WIDTH, _mod.RANGE_BASE


def git(*a):
    r = subprocess.run(["git", "-C", REPO, *a], capture_output=True)
    if r.returncode:
        raise SystemExit(f"git {a}: {r.stderr.decode()}")
    return r.stdout.decode()


def show(c, p):
    r = subprocess.run(["git", "-C", REPO, "show", f"{c}:{p}"], capture_output=True)
    return r.stdout.decode("utf-8") if r.returncode == 0 else None


def apply_para(text, rule, width):
    lines = text.split("\n")
    hits = [i for i, l in enumerate(lines) if rule["anchor"] in l]
    if len(hits) != 1:
        raise ValueError(f"anchor on {len(hits)} lines")
    i = hits[0]
    m = re.match(r"^(\s*// )(?! )", lines[i])
    if not m:
        raise ValueError(f"anchor line is not prose: {lines[i]!r}")
    prefix = m.group(1)

    def prose(l):
        return l.startswith(prefix) and not l[len(prefix):].startswith(" ") and l[len(prefix):].strip()

    a = i
    if not rule.get("from_anchor"):
        while a > 0 and prose(lines[a - 1]):
            a -= 1
    b = i
    stop = rule.get("stop_before")
    while b + 1 < len(lines) and prose(lines[b + 1]) and not (stop and stop in lines[b + 1]):
        b += 1
    text_p = " ".join(l[len(prefix):].strip() for l in lines[a:b + 1])
    for old, new in rule["subs"]:
        n = text_p.count(old)
        if n != 1:
            raise ValueError(f"prose sub occurs {n}x: {old[:60]!r}")
        text_p = text_p.replace(old, new)
    wrapped = textwrap.wrap(text_p, width=width - len(prefix), break_on_hyphens=False,
                            break_long_words=False)
    return "\n".join(lines[:a] + [prefix + w for w in wrapped] + lines[b + 1:])


def apply(text, rule):
    if rule["kind"] == "add":
        if text is not None:
            raise ValueError("add: the path already exists")
        return rule["content"]
    if rule["kind"] == "sub":
        n = text.count(rule["old"])
        if n != 1:
            raise ValueError(f"sub occurs {n}x: {rule['old'][:60]!r}")
        return text.replace(rule["old"], rule["new"])
    return apply_para(text, rule, WIDTH[rule["path"]])


def main():
    mode = sys.argv[1]
    commits = git("rev-list", "--reverse", f"{RANGE_BASE}..HEAD").split()
    pos = {c: i for i, c in enumerate(commits)}
    for r in RULES:
        full = git("rev-parse", r["frm"]).strip()
        if full not in pos:
            raise SystemExit(f"{r['id']}: owner {r['frm']} not in range")
        r["_pos"] = pos[full]
        r["_until"] = pos[git("rev-parse", r["until"]).strip()] if r.get("until") else len(commits)
    paths = sorted({r["path"] for r in RULES})
    applied = {r["id"]: 0 for r in RULES}
    errors = []
    out = []  # (commit, path, content)
    for c in commits:
        for p in paths:
            orig = show(c, p)
            live = [r for r in RULES
                    if r["path"] == p and r["_pos"] <= pos[c] < r["_until"]]
            if orig is None and not any(r["kind"] == "add" for r in live):
                continue
            t = orig
            for r in RULES:
                if r["path"] != p or pos[c] < r["_pos"] or pos[c] >= r["_until"]:
                    continue
                try:
                    t = apply(t, r)
                    applied[r["id"]] += 1
                except ValueError as e:
                    errors.append((r["id"], c[:7], str(e)))
            if t != orig:
                out.append((c, p, t))
    if mode == "dry":
        for r in RULES:
            need = r["_until"] - r["_pos"]
            flag = "OK " if applied[r["id"]] == need else "BAD"
            print(f"{flag} {r['id']:<12} owner {r['frm']}  applied {applied[r['id']]}/{need}")
        seen = set()
        for rid, c, e in errors:
            if (rid, e) in seen:
                continue
            seen.add((rid, e))
            n = sum(1 for x in errors if x[0] == rid and x[2] == e)
            print(f"  ERR {rid} first at {c} ({n} commits): {e}")
        print(f"{len(out)} (commit, path) blobs would change; {len(errors)} errors")
    elif mode == "tip":
        tip = commits[-1]
        for c, p, t in out:
            if c == tip:
                d = difflib.unified_diff((show(c, p) or "").split("\n"), t.split("\n"), f"a/{p}", f"b/{p}",
                                         lineterm="", n=1)
                print("\n".join(d))
    elif mode == "map":
        if errors:
            raise SystemExit("refusing: errors in dry run")
        with open(HERE / f"indexmap-{os.environ.get('RULES', 'rules1')}.txt", "w") as f:
            for c, p, t in out:
                blob = subprocess.run(["git", "-C", REPO, "hash-object", "-w", "--stdin"],
                                      input=t.encode(), capture_output=True, check=True).stdout.decode().strip()
                f.write(f"{c} 100644 {blob} {p}\n")
        print(f"wrote {len(out)} entries")


if __name__ == "__main__":
    main()
