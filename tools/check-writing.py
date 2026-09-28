#!/usr/bin/env python3
"""Gate for the writing rules in .notes/WRITING-RULES.md that a machine can check.

The rules were learned expensively and written down, and we still broke them -- three stale
headings, historical framing in a shipped comment, a fork message citing tests that do not sync.
Prose rules nobody runs are documentation, not a gate. This runs the greppable subset.

    check-writing.py comments <path>...     shipped source: history, dev SHAs, nicknames, long blocks;
                                            a CHANGELOG.md is read as its TOP section only, and also
                                            checked for pointers into the source
    check-writing.py comments-new <base> <root>   only what <root>'s tree ADDS over <base>'s
    check-writing.py forkmsg <dir>          fork messages: dev SHAs, tools/ paths, test claims
    check-writing.py headings <file>...     a heading whose own section contradicts it

The pattern lists carry a SELFTEST of phrasings that actually shipped, asserted on every run.

Exit 1 on any finding. Judgement rules -- "is this a constraint or an argument" -- are NOT here and
cannot be; see the checklist.
"""
import json
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
# Calibrated against the shipped tree 2026-09-28, when a whole-PR read found sixteen history
# comments this had passed. "no longer", "previously", "now that", "the old" and "predates" each
# have runtime uses here ("the card no longer answers", "a previously scanned tag"), so none is
# matched. "in the first place" and "historical" had only history uses. Like every entry, they are a
# tripwire for a phrasing coming back, not coverage: most history is not greppable.
HISTORY = re.compile(
    r"((?<!\bis )(?<!\bare )(?<!\bwas )(?<!\bwere )(?<!\bbeen )(?<!\bbeing )\bused to\b"
    r"|\bwas previously\b|\bpreviously (?:set|called|named|spelled|lived|sat|disagreed|read)\b"
    r"|\bearlier (?:version|draft|commit)\b"
    r"|\bwe (?:cut|removed|added|broke|restored|had)\b|\bthis (?:used|had) to\b"
    r"|\bbefore (?:the|this) (?:cut|trim|round|change)\b"
    r"|\bin the first place\b|\bhistorical\b)", re.I)

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


# A CARD'S BENCH NAME IS NOT A NAME ANY READER HAS. The shelf's nicknames -- the hyphenated keys of
# tools/tag-inventory.json -- leaked into shipped comments and were cut 2026-09-27. Matched on any
# line, code and strings included, since a nickname belongs in neither. Unhyphenated keys are part
# numbers (SL2S5302), which are real names, so they are not matched.
def _nicknames():
    try:
        data = json.loads((Path(__file__).resolve().parent / "tag-inventory.json").read_text())
    except (OSError, ValueError):
        return None
    tags = data.get("tags", data) if isinstance(data, dict) else {}
    names = [k for k in tags if "-" in k]
    if not names:
        return None
    return re.compile(r"(?<![\w-])(" + "|".join(map(re.escape, names)) + r")(?![\w-])", re.I)


NICKNAME = _nicknames()

# A RELEASE NOTE IS READ BY SOMEONE USING THE APP, who cannot act on a pointer into the source: 2.3
# shipped "the closed form is stated in the poller beside that branch". Checked in the top release
# section of CHANGELOG.md only -- internal vocabulary, file names, snake_case identifiers.
INTERNAL = re.compile(r"\bpoller\b|\b\w+\.[ch]\b|\b[a-z0-9]+(?:_[a-z0-9]+){2,}\b")


def trailing_comment(line):
    """The text after a `//` that follows code on the same line, or None.

    The gate read only lines that START with `//`, so the 50-odd comments trailing a field or a
    statement were never scanned. String and character literals are tracked so that a `//` inside
    one -- a URL, a format string -- is not taken for a comment."""
    quote, i = None, 0
    while i < len(line) - 1:
        c = line[i]
        if quote:
            if c == "\\":
                i += 2
                continue
            if c == quote:
                quote = None
        elif c in "\"'":
            quote = c
        elif c == "/" and line[i + 1] == "/":
            return line[i + 2:]
        i += 1
    return None


def prose(p, i, s):
    out = []
    if HISTORY.search(s):
        out.append((p, i, "history", s.strip()[:70]))
    # A quoted example is a value, not a reference: "E0040150 12345678" is a UID format.
    quoted = [m.span() for m in re.finditer(r'"[^"]*"', s)]
    for m in DEV_SHA.finditer(s):
        # hex that is plausibly a SHA rather than a block number or a mask
        if re.search(r"0x", s[max(0, m.start() - 2):m.start()]):
            continue
        if not any(a <= m.start() < b for a, b in quoted):
            out.append((p, i, "dev-sha?", m.group()))
    return out


def nickname(p, i, line):
    m = NICKNAME.search(line) if NICKNAME else None
    return [(p, i, "nickname", m.group() + " -- a bench name; say what the card is")] if m else []


def release_notes(p, lines):
    """The TOP section of a CHANGELOG -- the release being written -- read as prose. The older
    sections are other people's releases, written to a changelog's own conventions."""
    starts = [i for i, l in enumerate(lines) if l.startswith("## ")]
    if not starts:
        return []
    out = []
    for i in range(starts[0], starts[1] if len(starts) > 1 else len(lines)):
        out += prose(p, i + 1, lines[i]) + nickname(p, i + 1, lines[i])
        m = INTERNAL.search(lines[i])
        if m:
            out.append((p, i + 1, "internal", f"{m.group()!r} -- a user cannot act on the source"))
    return out


def comments(paths):
    bad, warn = [], []
    for p in paths:
        lines = Path(p).read_text().split("\n")
        if str(p).endswith(".md"):
            bad += release_notes(p, lines)
            continue
        run, start = 0, 0
        for i, line in enumerate(lines, 1):
            s = line.strip()
            bad += nickname(p, i, line)
            if s.startswith("//"):
                if run == 0:
                    start = i
                run += 1
                bad += prose(p, i, s)
            else:
                if run > BLOCK_WARN:
                    warn.append((p, start, "long-block", f"{run} lines -- justify or cut"))
                run = 0
                t = trailing_comment(line)
                if t is not None:
                    bad += prose(p, i, "//" + t)
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
        if (Path(root) / "CHANGELOG.md").exists():
            files.append(str(Path(root) / "CHANGELOG.md"))
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
    files = sorted(Path(d).glob("[0-9][0-9]-*.msg"))
    # A directory with no NN-*.msg in it scanned nothing, and "0 findings" would read as clean.
    if not files:
        bad.append((str(d), 0, "no-files", "no NN-*.msg here -- nothing was scanned"))
    for f in files:
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
    (HISTORY, True, "// It never aborted a write in the first place -- leaving this scene runs on_exit"),
    (HISTORY, True, "// The `clone_` prefix is historical: a wipe reuses the same fields"),
    (HISTORY, False, "// carry an address the card no longer answers to"),
    (HISTORY, False, "// Fresh scan: drop any password / wipe mode armed for a previously scanned tag"),
    (HISTORY, False, "// So re-probe the run now that the card is known to be answering"),
    (INTERNAL, True, "and the closed form is stated in the poller beside that branch: a card"),
    (INTERNAL, False, "a card that answers a read at every address walks past 56/57 whatever it claims"),
]
if NICKNAME:
    SELFTEST += [(NICKNAME, True, "benched on lri2k-keychain 2026-09-28"),
                 (NICKNAME, False, "an ST LRi2K, 56 blocks, and an NXP SL2S5302")]

# The comment extractor, pinned the same way: each of these is a line shape the shipped tree has.
TRAILING_SELFTEST = [
    ("    bool uid_changed; // set only from a positive observation", " set only from a positive observation"),
    ('    furi_string_cat_str(s, "see http://x");', None),
    ('    s = "a // b"; // the real one', " the real one"),
    ("    c = '/'; // after a char literal", " after a char literal"),
    ('    s = "a \\" // still in the string";', None),
]


def selftest():
    bad = [t for pat, want, t in SELFTEST if bool(pat.search(t)) != want]
    bad += [line for line, want in TRAILING_SELFTEST if trailing_comment(line) != want]
    for s, want in [('// "E0040150 12345678" -- 17 chars', False), ("// moved to 1b8cea0", True)]:
        if bool([f for f in prose("-", 0, s) if f[2] == "dev-sha?"]) != want:
            bad.append(s)
    if NICKNAME is None:
        print("  warn nickname    tools/tag-inventory.json unreadable -- nicknames NOT checked")
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
