#!/usr/bin/env python3
"""Rebuild REVIEW-ALL.md from the individual drafts, so the review copy can't drift
from what post-pr-replies.sh actually posts. Run after editing any draft."""
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
THREADS = [
    ("3719022267", "magic/protocols/iso15693/iso15693_poller.c:897", "Blocking 1 — the read-back can't see the change it exists to catch", "FIXED"),
    ("3719022269", "magic/protocols/iso15693/iso15693_poller.c:730", "Blocking 2 — the sweep can cover less than the loop it replaced", "FIXED"),
    ("3719022271", "magic/protocols/iso15693/iso15693_poller.c:786", "Blocking 3 — a truncated sweep and a complete one are indistinguishable", "FIXED"),
    ("3719022293", "magic/protocols/iso15693/iso15693_poller.c:657", "No wall-clock bound on the sweep", "FIXED"),
    ("3719022325", "CHANGELOG.md:85", "Stale “and the UID is unchanged”", "FIXED"),
    ("3719022273", "magic/protocols/iso15693/iso15693_poller.c:762", "Re-probe classifies by presence, main loop by content", "NOT fixed — replying because this round made it worse"),
    ("3719022322", "scenes/nfc_magic_scene_iso15693_write_fail.c:131", "“10 render branches”", "NOT fixed — replying because it’s now 11"),
]
ISSUES = ["issue-1-unaddressed-frames.md", "issue-2-poller-timeouts.md", "issue-3-write-check-back-loop.md"]
out = []
w = out.append
w("# PR #250 — everything queued for posting\n")
w("Generated from the files in this directory by `assemble-review.py`. Nothing here has been posted.\n"
  "Push the fork branch **first** — the replies cite fork SHAs.\n")
w("| # | goes to | source |")
w("|---|---|---|")
w("| 1 | PR #250 top-level comment | `pr-reply-body.md` |")
for i, (cid, loc, _, _) in enumerate(THREADS, start=2):
    w(f"| {i} | thread on `{loc}` | `pr-inline/{cid}.md` |")
for i, f in enumerate(ISSUES, start=2 + len(THREADS)):
    w(f"| {i} | new issue — {open(f).readline().lstrip('# ').strip()[:58]}… | `{f}` |")
w("\n---\n\n# 1. Top-level comment on PR #250\n")
w(open("pr-reply-body.md").read().rstrip())
for cid, loc, what, st in THREADS:
    w(f"\n---\n\n# Reply in thread: `{loc}`\n")
    w(f"**His comment:** {what}  \n**Status:** {st}  \n**Thread id:** `{cid}`\n")
    w(open(f"pr-inline/{cid}.md").read().rstrip())
for f in ISSUES:
    b = open(f).read().rstrip().splitlines()
    w("\n---\n\n# New issue\n")
    w(f"**Title:** {b[0].lstrip('# ').strip()}\n")
    w("\n".join(b[1:]).strip())
open("REVIEW-ALL.md", "w").write("\n".join(out) + "\n")
print("wrote REVIEW-ALL.md")
