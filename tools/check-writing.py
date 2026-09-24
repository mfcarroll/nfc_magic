#!/usr/bin/env python3
"""Gate for the writing rules in .notes/WRITING-RULES.md that a machine can check.

The rules were learned expensively and written down, and we still broke them -- three stale
headings, historical framing in a shipped comment, a fork message citing tests that do not sync.
Prose rules nobody runs are documentation, not a gate. This runs the greppable subset.

    check-writing.py comments <path>...     shipped source: history, dev SHAs, long blocks
    check-writing.py forkmsg <dir>          fork messages: dev SHAs, tools/ paths, test claims
    check-writing.py headings <file>...     a heading whose own section contradicts it

Exit 1 on any finding. Judgement rules -- "is this a constraint or an argument" -- are NOT here and
cannot be; see the checklist.
"""
import re
import sys
from pathlib import Path

# Framing that describes how the code GOT here rather than what it now requires. A comment is read
# by someone deciding whether they may change the line in front of them; its history is not their
# problem and goes stale silently.
# Deliberately narrow. An over-reporting check gets ignored, which is its own recorded lesson: these
# are phrases that can ONLY be about the edit history, never about the card or the code's behaviour.
# "no longer inventories" and "every block was written" are present-tense and must not match.
HISTORY = re.compile(
    r"(\bused to\b|\bwas previously\b|\bpreviously (?:set|called|named|spelled|lived|sat|disagreed|read)\b"
    r"|\bearlier (?:version|draft|commit)\b"
    r"|\bwe (?:cut|removed|added|broke|restored|had)\b|\bthis (?:used|had) to\b"
    r"|\bbefore (?:the|this) (?:cut|trim|round|change)\b)", re.I)
DEV_SHA = re.compile(r"\b[0-9a-f]{7,9}\b")
BLOCK_WARN = 20  # a comment run longer than this wants a reason


def comments(paths):
    bad, warn = [], []
    for p in paths:
        lines = Path(p).read_text().split("\n")
        run, start = 0, 0
        for i, line in enumerate(lines, 1):
            s = line.strip()
            if s.startswith("//"):
                if run == 0:
                    start = i
                run += 1
                if HISTORY.search(s):
                    bad.append((p, i, "history", s[:70]))
                for m in DEV_SHA.finditer(s):
                    # hex that is plausibly a SHA rather than a block number or a mask
                    if not re.search(r"0x", s[max(0, m.start() - 2):m.start()]):
                        bad.append((p, i, "dev-sha?", m.group()))
            else:
                if run > BLOCK_WARN:
                    warn.append((p, start, "long-block", f"{run} lines -- justify or cut"))
                run = 0
    return bad, warn


def forkmsg(d):
    bad = []
    for f in sorted(Path(d).glob("[0-9][0-9]-*.msg")):
        for i, line in enumerate(f.read_text().split("\n"), 1):
            if re.search(r"\btools/|hosttest|\btests? (?:pass|pin|cover)\b|\bmutation-check", line, re.I):
                bad.append((f.name, i, "unsyncable", line.strip()[:70]))
            for m in DEV_SHA.finditer(line):
                bad.append((f.name, i, "dev-sha?", m.group()))
    return bad


# A heading goes stale because the body is edited and the title is not -- four times in four rounds
# here. Chasing that with contradiction-detection failed: a check for "pending" vs "benched" passed
# clean while a heading said "two of the behavioural three" over a body saying all three.
#
# So the rule is not "keep them in sync", it is: A HEADING STATES ITS SUBJECT, NEVER A COUNT OR A
# STATUS. "Your twelve, and what the bench says" cannot rot. "Two of three benched" rots the moment
# the third is run. That IS mechanically checkable, which the contradiction was not.
COUNTY = re.compile(r"\b(all|none|both|one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|\d+)\b"
                    r"|\b(pending|outstanding|remaining|so far|still to|owed|unbenched)\b", re.I)


def headings(paths):
    bad = []
    for p in paths:
        for i, line in enumerate(Path(p).read_text().split("\n"), 1):
            if not line.startswith("## "):
                continue
            # A thread heading is an anchor -- `CHANGELOG.md:99 -- comment 4035110593`. Those digits
            # are references, not claims, and they have to be exact. Strip them and code spans first.
            probe = re.sub(r"`[^`]*`", "", line)
            probe = re.sub(r"\S+\.\w+:\d+|comment \d+|#\d+|0x[0-9a-fA-F]+", "", probe)
            m = COUNTY.search(probe)
            if m:
                bad.append((p, i, "heading-count", f"{m.group()!r} in a heading -- state the subject"))
    return bad


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    mode, args = sys.argv[1], sys.argv[2:]
    res = {"comments": comments, "forkmsg": lambda a: (forkmsg(a[0]), []),
           "headings": lambda a: (headings(a), [])}[mode](args)
    found, warn = res
    for where, line, kind, detail in warn:
        print(f"  warn {kind:<11} {where}:{line}  {detail}")
    for where, line, kind, detail in found:
        print(f"  FAIL {kind:<11} {where}:{line}  {detail}")
    print(f"{len(found)} finding(s), {len(warn)} warning(s)")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
