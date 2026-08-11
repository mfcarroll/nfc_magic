#!/usr/bin/env python3
"""Rebuild REVIEW-ALL.md from the individual drafts, so the review copy can't drift
from what post-pr-replies.sh actually posts. Run after editing any draft."""
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
THREADS = [
    ('3756085157', 'iso15693_poller.c:859', 'Blocking -- tail-drop vs the advertised-count floor', 'FIXED'),
    ('3756085164', 'iso15693_poller.c:978', 'sweep_truncated never reaches the nothing-wiped screen', 'FIXED'),
    ('3756085182', 'iso15693_poller.c:532', 'the clone data pass got the Back swallow but not the clock', 'FIXED'),
    ('3756085187', 'iso15693_poller.c:1125', 'VerifyWipe asserts a check it skipped', 'FIXED'),
    ('3756085228', 'scenes/nfc_magic_scene_write.c:519', "two supporting sentences in the Back comment don't hold", 'FIXED'),
    ('3756085255', 'iso15693_poller.c:741', 'three copies of the block-answered rule', 'DONE -- asked for in this pass'),
    ('3756085261', 'scenes/nfc_magic_scene_iso15693_write_fail.c:344', 'the details gate stated twice, already disagreeing', 'DONE -- asked for in this pass'),
    ('3756085272', 'iso15693_poller.c:866', "'every exit' isn't", 'FIXED -- and it found a bug'),
]
ISSUES = []
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
