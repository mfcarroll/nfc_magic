#!/usr/bin/env python3
"""Gate for the writing rules in .notes/WRITING-RULES.md that a machine can check.

The rules were learned expensively and written down, and we still broke them -- three stale
headings, historical framing in a shipped comment, a fork message citing tests that do not sync.
Prose rules nobody runs are documentation, not a gate. This runs the greppable subset.

    check-writing.py comments <path>...     shipped source: history, dev SHAs, long blocks
    check-writing.py comments-new <base> <root>   only what <root>'s tree ADDS over <base>'s
    check-writing.py forkmsg <dir>          fork messages: dev SHAs, tools/ paths, test claims
    check-writing.py headings <file>...     a heading whose own section contradicts it

The pattern lists carry a SELFTEST of phrasings that actually shipped, asserted on every run.

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
# The lookbehinds on "used to" are load-bearing: "storage IS used to look up the key cache" is
# present-tense passive and was reported as history for a line nobody here wrote. A checker that
# cries wolf on the neighbours' code is one people stop reading, which is the same lesson as the
# staleness scanner.
HISTORY = re.compile(
    r"((?<!\bis )(?<!\bare )(?<!\bwas )(?<!\bwere )(?<!\bbeen )(?<!\bbeing )\bused to\b"
    r"|\bwas previously\b|\bpreviously (?:set|called|named|spelled|lived|sat|disagreed|read)\b"
    r"|\bearlier (?:version|draft|commit)\b"
    r"|\bwe (?:cut|removed|added|broke|restored|had)\b|\bthis (?:used|had) to\b"
    r"|\bbefore (?:the|this) (?:cut|trim|round|change)\b)", re.I)

# AN INFERENCE ABOUT A PERSON, WRITTEN AS A REPORT OF WHAT THEY SAID. Three times now, twice about
# the same issue: the round-15 reply told mishamyte #255 would be "less hypothetical than when YOU
# filed it" when mfcarroll filed it, and then a session note said #255 "is not ours to edit" for the
# same reason. WRITING-RULES has carried "mfcarroll filed it" in as many words throughout.
#
# A machine cannot check who said what. It CAN refuse to let the sentence through unexamined, which
# is the whole of the fix: every hit has to be answered with where it is recorded, or rewritten to
# say what is actually known ("filed as #255"). First person is not matched -- we are our own source.
ATTRIBUTION = re.compile(
    r"\b(you|he|she|they|mishamyte|mfcarroll"
    r"|the (?:sender|author|reporter|reviewer|maintainer|filer|owner|seller|submitter))"
    r"\s+(?:had\s+|have\s+|has\s+|first\s+|never\s+)?"
    r"(filed|raised|asked|said|called|reported|opened|wrote|flagged|requested|suggested"
    r"|described|named|claimed|believes?|wants?|thinks?|felt|meant)\b", re.I)
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


def comments_new(args):
    """Findings in the ROOT tree that the BASE tree does not already have.

    A replay ships every sync point's tree, not just the tip, so each one is gated -- but an
    intermediate tree can carry a finding it did not introduce, one already on the branch that a
    later sync point in the same push removes. Gating those would refuse a correction for the text
    it corrects. So this reports only what the tree ADDS: matched on (path within its tree, kind,
    text) as a multiset, since line numbers move between trees and the same phrase can occur twice.
    """
    from collections import Counter

    def scan(root):
        files = sorted(str(p) for p in Path(root).rglob("*.[ch]"))
        found, _ = comments(files)
        return [(str(Path(p).relative_to(root)), i, k, t) for p, i, k, t in found]

    have = Counter((p, k, t) for p, _, k, t in scan(args[0]))
    new = []
    for p, i, k, t in scan(args[1]):
        if have[(p, k, t)]:
            have[(p, k, t)] -= 1
        else:
            new.append((p, i, k, t))
    return new, []


def forkmsg(d):
    bad = []
    for f in sorted(Path(d).glob("[0-9][0-9]-*.msg")):
        for i, line in enumerate(f.read_text().split("\n"), 1):
            if re.search(r"\btools/|hosttest|\btests? (?:pass|pin|cover)\b|\bmutation-check", line, re.I):
                bad.append((f.name, i, "unsyncable", line.strip()[:70]))
            for m in DEV_SHA.finditer(line):
                bad.append((f.name, i, "dev-sha?", m.group()))
            m = ATTRIBUTION.search(line)
            if m:
                bad.append((f.name, i, "attributed", m.group() + " -- where is that recorded?"))
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


# Asserted on EVERY invocation, and the gate refuses to run if any of it fails. The precedent is
# tools/gen1-staleness.py, whose pattern list was written by reading the sites already fixed and was
# therefore blind to all eight that survived -- and reported the job done. A pattern list needs
# cases it MUST match and cases it MUST NOT, or its silence means nothing. Every line here is a
# phrasing that actually shipped or was actually reported.
SELFTEST = [
    (HISTORY, False, "// storage is used to look up the NFC app's per-UID MIFARE Classic key"),
    (HISTORY, False, "// the two buffers are used to hold the frame and its answer"),
    (HISTORY, True, "// it used to take whatever the inventory returned"),
    (HISTORY, True, "// we cut the cost footnote"),
    (ATTRIBUTION, True, "less hypothetical than when you filed it"),
    (ATTRIBUTION, True, "a coin the sender called locked"),
    (ATTRIBUTION, True, "He never called it locked"),
    (ATTRIBUTION, True, "the seller said those are the proxmark-writable ones"),
    (ATTRIBUTION, True, "he wants the gen3 probe as its own PR"),
    (ATTRIBUTION, False, "I told you TI Tag-it refuses an unaddressed WRITE BLOCK"),
    (ATTRIBUTION, False, "the card asked for the OPTION flag"),
    (ATTRIBUTION, False, "filed as #255"),
    (ATTRIBUTION, False, "described as a V1 specimen"),
]


def selftest():
    bad = [t for pat, want, t in SELFTEST if bool(pat.search(t)) != want]
    if bad:
        print("SELFTEST FAILED -- the patterns do not do what they claim; refusing to scan:")
        for t in bad:
            print("   " + t)
    return not bad


def main():
    if not selftest():
        return 2
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    mode, args = sys.argv[1], sys.argv[2:]
    res = {"comments": comments, "comments-new": comments_new,
           "forkmsg": lambda a: (forkmsg(a[0]), []),
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
