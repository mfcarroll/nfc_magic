#!/usr/bin/env python3
"""Verify every fork SHA cited in the drafts still exists on the fork branch.
Run before posting -- the stack has been restructured several times and a stale
citation is a dead link in a public reply."""
import glob, os, re, subprocess, sys
os.chdir(os.path.dirname(os.path.abspath(__file__)))
FORK, BASE = "../../../all-the-plugins", "a3e13a3a"
try:
    fork = set(subprocess.run(["git", "log", "--format=%h", f"{BASE}..HEAD"],
                              cwd=FORK, capture_output=True, text=True, check=True).stdout.split())
except Exception as e:
    sys.exit(f"could not read the fork at {FORK}: {e}")
cited = {}
for f in ["pr-reply-body.md"] + sorted(glob.glob("pr-inline/*.md")):
    for sha in re.findall(r"`([0-9a-f]{8})`", open(f).read()):
        cited.setdefault(sha, []).append(f)
stale = {s: v for s, v in cited.items() if s not in fork}
for s, files in sorted(stale.items()):
    print(f"STALE {s} -> {', '.join(files)}")
print(f"{len(cited)} distinct SHAs cited, {len(stale)} stale")
sys.exit(1 if stale else 0)
