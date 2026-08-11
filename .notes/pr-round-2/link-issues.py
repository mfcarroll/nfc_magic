#!/usr/bin/env python3
"""Substitute real GitHub issue numbers into the drafts, once the issues are filed.

    ./link-issues.py <frames#> <stall#> <backloop#>       e.g. ./link-issues.py 118 119 120

Order matches the manifest: unaddressed frames, poller stall, write-check Back loop.
Run after filing the issues and before posting the PR comment, so the body links live
issues instead of describing them. Re-runnable: it only touches unsubstituted tokens.
"""
import glob, os, re, sys
os.chdir(os.path.dirname(os.path.abspath(__file__)))
if len(sys.argv) != 4:
    sys.exit(__doc__)
nums = []
for a in sys.argv[1:]:
    a = a.lstrip("#")
    if not a.isdigit():
        sys.exit(f"not an issue number: {a}")
    nums.append(a)
M = dict(zip(["#ISSUE-FRAMES", "#ISSUE-STALL", "#ISSUE-BACKLOOP"], [f"#{n}" for n in nums]))
touched = 0
for f in ["pr-reply-body.md"] + sorted(glob.glob("issue-*.md")) + sorted(glob.glob("pr-inline/*.md")):
    s = open(f).read()
    o = s
    for tok, num in M.items():
        s = s.replace(tok, num)
    if s != o:
        open(f, "w").write(s)
        print(f"linked {f}")
        touched += 1
print(f"{touched} file(s) updated")
print("\nNote: the two issue bodies cross-reference each other, so if you want those links live "
      "you must paste the updated bodies back into the filed issues. The PR comment is the one "
      "that matters -- it is posted after this runs.")
