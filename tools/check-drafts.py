#!/usr/bin/env python3
"""Verify every reference in a PR-reply draft before it is posted.

Nothing in the build, test or format pipeline reads prose, so a draft can cite a commit
that no longer exists, a line number that points at nothing, a file that was renamed, or
an identifier that was deleted -- and every check will pass. This is that check.

What it verifies:
  SHAs         -- any 7-9 hex token. A DEV-repo SHA is ALWAYS wrong in a PR comment:
                  sync-to-fork.sh overlays its own commits, so the branch the reviewer
                  reads has different hashes. Rewritten ones are worse -- they resolve
                  locally and not for him.
  line refs    -- `file.c:NNN` and bare `:NNN`. Checked against every anchor the reviewer
                  has ever left on the PR (cached in .notes/pr-round-*/received/), because
                  a reply's line ref means HIS thread location, not our file. Bare `:NNN`
                  is flagged regardless: it does not say which file.
  files        -- referenced basenames must exist in the tree.
  identifiers  -- snake_case with 2+ underscores, or Iso15693*/NfcMagic* CamelCase, must
                  appear somewhere in the shipped source.
  issues       -- #NNN must exist; reports issue-vs-PR and open/closed so a draft cannot
                  call a PR an issue.

Usage:  python3 tools/check-drafts.py .notes/pr-round-7/*.md .notes/pr-description.md
        Where ~~~~ markers delimit the text that actually gets posted, only that payload counts
        toward the verdict; preamble hits are reported as [PREAMBLE, not posted].
        (add --anchors to dump every anchor the reviewer has left, for cross-checking)

Line numbers are not stable identifiers in either direction -- GitHub re-anchors a thread
as the file moves, so one thread can read :155 in the API and :175 in his own prose. When
in doubt cite the finding, not the line; thread-replies.md's own header says so.
"""
import re, sys, os, json, glob, subprocess

# Sibling firmware checkouts. A SHA that resolves in one of these is an external reference the
# reviewer CAN look up -- unlike one from this repo, which he can never resolve.
FW_TREES = [p for p in glob.glob(os.path.expanduser("~/../Shared/code/personal/rfid/*"))
            + glob.glob("../*") if os.path.isdir(os.path.join(p, ".git"))]

# AN INFERENCE ABOUT A PERSON, WRITTEN AS A REPORT OF WHAT THEY SAID -- the one class of fact that
# cannot be reconstructed from context, and the only rule in WRITING-RULES that has now been broken
# three times. Twice about #255 alone: a reply told mishamyte it would be "less hypothetical than
# when YOU filed it", and a later note said the issue "is not ours to edit", both when mfcarroll
# filed it and WRITING-RULES said so in as many words.
#
# No checker can know who said what. This one refuses to let the sentence through unexamined, which
# is the entire fix: answer each hit with where it is recorded, or rewrite it to say what is
# actually known -- "filed as #255", "described as a V1 specimen". First person is not matched; we
# are our own source for what we told him.
ATTRIBUTION = re.compile(
    r"\b(you|he|she|they|mishamyte|mfcarroll"
    r"|the (?:sender|author|reporter|reviewer|maintainer|filer|owner|seller|submitter))"
    r"\s+(?:had\s+|have\s+|has\s+|first\s+|never\s+)?"
    r"(filed|raised|asked|said|told|called|reported|opened|wrote|flagged|requested|suggested"
    r"|described|named|claimed|believes?|wants?|thinks?|felt|meant)\b", re.I)

# NARRATION OF OUR OWN PROCESS, wearing the clothes of a finding. WRITING-RULES has the test -- is
# this passage about the CODE and the decision, or about us: what we tried, what we learned, how we
# feel about it. mfcarroll has now caught four of these by eye in one round: "yours is better than
# what I had written", "the re-address checks its answer NOW", "we did build it to see what it would
# cost", and "which I can now say rather than infer". Every one was true, well-written, and about
# the wrong subject.
#
# Deliberately narrow: first-person discovery and surprise, which a reply almost never needs. A
# commit message MAY legitimately say what its own diff changed, so this runs on reply payloads only.
NARRATION = re.compile(
    r"\b(I (did ?n[o']t|could ?n[o']t) (expect|see|tell)"
    r"|I can now\b|rather than infer\b|I had (written|recorded|assumed)"
    r"|turn(s|ed) out to\b|as it happens\b|it emerged\b"
    r"|we (did |had )?(built|tried|ran) (it|this|that)"
    r"|to (see|find out) what it would cost)", re.I)

NARRATION_SELFTEST = [
    (True, "I did not expect that, and it means the sequence had been going out"),
    (True, "which I can now say rather than infer"),
    (True, "That tag also turned out to keep a writable UID register"),
    (True, "We did build it, to see what it would cost"),
    (True, "yours is better than what I had written"),
    (False, "the card answers its writes directly"),
    (False, "a UID one byte wrong gets nothing at all"),
    (False, "the survey reads upward from the source count"),
]

ATTRIBUTION_SELFTEST = [
    (True, "less hypothetical than when you filed it"),
    (True, "The seller told me there is no support for it in proxmark"),
    (True, "a coin the sender called locked"),
    (True, "He never called it locked"),
    (True, "he wants the gen3 probe as its own PR"),
    (False, "I told you TI Tag-it refuses an unaddressed WRITE BLOCK"),
    (False, "the card asked for the OPTION flag"),
    (False, "filed as #255"),
]


def sh(*a):
    return subprocess.run(a, capture_output=True, text=True).stdout

def anchors():
    """Every line the reviewer has anchored a comment at, from the cached rounds."""
    out = {}
    # The cached rounds are not one format: some are a JSON array, some JSONL.
    for f in glob.glob(".notes/pr-round-*/received/*.json"):
        body = open(f, encoding="utf-8").read()
        items = []
        try:
            d = json.loads(body)
            items = d if isinstance(d, list) else [d]
        except ValueError:
            for line in body.split("\n"):
                line = line.strip()
                if not line:
                    continue
                try:
                    items.append(json.loads(line))
                except ValueError:
                    pass
        for t in items:
            if not isinstance(t, dict):
                continue
            p = (t.get("path") or "").split("/")[-1]
            ln = t.get("line") or t.get("original_line")
            if p and ln:
                out.setdefault(p, set()).add(int(ln))
    # The cache holds only the rounds that were captured, and GitHub RE-ANCHORS a thread as the
    # file moves -- so a thread can read :155 in the API and :175 in his own prose, and neither
    # number is wrong. Merge both sources and treat a miss as unconfirmable, not as invalid.
    live = ".notes/pr-anchors.json"
    got = None
    if os.path.exists(live):
        got = json.load(open(live))
    else:
        # --paginate with an array-wrapping --jq emits ONE ARRAY PER PAGE, which will not parse
        # as a single document. Emit one object per line instead and read it as JSONL.
        raw = sh("gh", "api", "--paginate",
                 "repos/xMasterX/all-the-plugins/pulls/250/comments?per_page=100",
                 "--jq", '.[] | {p: (.path|split("/")|last), l: (.line // .original_line)}')
        got = {}
        for line in (raw or "").split("\n"):
            line = line.strip()
            if not line:
                continue
            try:
                e = json.loads(line)
            except ValueError:
                continue
            if e.get("p") and e.get("l"):
                got.setdefault(e["p"], []).append(e["l"])
        if got:
            json.dump({k: sorted(set(v)) for k, v in got.items()},
                      open(live, "w"), indent=1, sort_keys=True)
    for k, v in (got or {}).items():
        out.setdefault(k, set()).update(int(x) for x in v)
    return out

def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    tree = sh("git", "ls-files").split()
    base = {os.path.basename(f): f for f in tree}
    src = "\n".join(open(f, encoding="utf-8", errors="replace").read()
                    for f in tree if f.endswith((".c", ".h")))
    anch = anchors()
    if "--anchors" in sys.argv:
        for p in sorted(anch):
            print("  %-46s %s" % (p, " ".join(str(x) for x in sorted(anch[p]))))
        return 0
    fails = [t for want, t in ATTRIBUTION_SELFTEST if bool(ATTRIBUTION.search(t)) != want]
    fails += [t for want, t in NARRATION_SELFTEST if bool(NARRATION.search(t)) != want]
    if fails:
        print("SELFTEST FAILED -- a pattern does not do what it claims:")
        for t in fails:
            print("   " + t)
        return 2

    bad = 0
    for path in args:
        txt = open(path, encoding="utf-8").read()
        lines = txt.split("\n")
        # These drafts carry a notes preamble above the text that actually gets posted. Where
        # ~~~~ markers delimit a payload, only that payload is POSTED -- a dev SHA in the
        # preamble is a working note, not a defect. Report both, scope the verdict to the payload.
        fences = [i for i, l in enumerate(lines, 1) if l.strip() == "~~~~"]
        payload = set()
        if len(fences) >= 2:
            for a, b in zip(fences[::2], fences[1::2]):
                payload.update(range(a, b + 1))
        else:
            payload = set(range(1, len(lines) + 1))
        print("##### %s%s" % (path, "" if len(fences) < 2
                              else "   (payload: %d of %d lines)" % (len(payload), len(lines))))
        # Hex inside a FENCED BLOCK is command transcript, not a hash -- `-d 11223344` is a write
        # payload and `E0 04 01 50` is a UID. Flagging those trains the reader to skim the report,
        # which is the same failure as a build that always prints warnings. Prose outside the fences
        # is still scanned, and that is where a dead sha would actually be cited.
        in_code = set()
        code = False
        for i, l in enumerate(lines, 1):
            if l.lstrip().startswith("```"):
                code = not code
                in_code.add(i)
            elif code:
                in_code.add(i)

        # [U+1F464] marks a paragraph mfcarroll wrote himself, posted as-is. The attribution and
        # narration rules exist to stop THIS process asserting what someone said or narrating its own
        # discovery; in his own paragraph he is the first-hand source and the narrator by right. The
        # factual checks -- SHAs, line refs, files, identifiers, issues -- still apply, because a dead
        # reference is dead whoever typed it.
        his = set()
        marked = False
        for i, l in enumerate(lines, 1):
            if l.lstrip().startswith("[\U0001F464]"):
                marked = True
            elif not l.strip():
                marked = False
            if marked:
                his.add(i)

        notes_only = 0
        for i, l in enumerate(lines, 1):
            def flag(kind, what, why):
                nonlocal bad, notes_only
                if i not in payload:
                    notes_only += 1
                    print("  :%-4d %-11s %-30s %s  [PREAMBLE, not posted]" % (i, kind, what, why))
                    return
                bad += 1
                print("  :%-4d %-11s %-30s %s" % (i, kind, what, why))

            for sha in (() if i in in_code else
                        re.findall(r'(?<![0-9a-zA-Z])[0-9a-f]{7,9}(?![0-9a-zA-Z])', l)):
                # Do NOT skip all-digit tokens: 7883953 was a real rewritten commit in a real
                # draft, and skipping it is how it survived a sweep. git is the decisive test --
                # a plain number does not resolve as a commit, so there is no false-positive cost.
                if subprocess.run(["git", "cat-file", "-e", sha + "^{commit}"],
                                  capture_output=True).returncode == 0:
                    flag("SHA", sha, "resolves locally -- a DEV sha is never valid in a PR comment")
                elif any(subprocess.run(["git", "-C", t, "cat-file", "-e", sha + "^{commit}"],
                                        capture_output=True).returncode == 0 for t in FW_TREES):
                    pass    # a FIRMWARE sha: legitimate, and he can look it up in that repo
                else:
                    flag("SHA?", sha, "resolves in no repo we know of; if meant as a hash it is dead")

            m = ATTRIBUTION.search(l) if (i not in in_code and i not in his) else None
            if m:
                flag("attributed", m.group()[:30],
                     "a claim about what a PERSON said -- find the record or say what is known")

            m = NARRATION.search(l) if (i not in in_code and i not in his) else None
            if m:
                flag("narration", m.group()[:30],
                     "about US rather than the code -- state the fact, not the discovery")

            for m in re.finditer(r'`([A-Za-z0-9_]+\.[ch])[:.](\d+)`', l):
                f, n = m.group(1), int(m.group(2))
                hits = [k for k in anch if k.endswith(f)]
                if not hits:
                    flag("line-ref", "%s:%d" % (f, n), "he has never commented on this file")
                elif not any(n in anch[k] for k in hits):
                    near = sorted(x for k in hits for x in anch[k] if abs(x - n) <= 20)
                    flag("line-ref?", "%s:%d" % (f, n),
                         "unconfirmed (anchors drift); nearest %s -- prefer citing the finding"
                         % (near or "none within 20"))

            for m in re.finditer(r'`:(\d+)`', l):
                flag("bare-ref", ":" + m.group(1), "names no file -- say which, or cite the finding")

            for m in re.finditer(r'`([A-Za-z0-9_]+\.(?:c|h|py|md|json|fam))`', l):
                nm = m.group(1)
                if nm not in base and not any(b.endswith(nm) for b in base):
                    flag("file", nm, "no file in the tree ends with this")

            for m in re.finditer(r'`([a-z][a-z0-9]*(?:_[a-z0-9]+){2,})`', l):
                if m.group(1) not in src:
                    flag("ident", m.group(1), "not found in any .c/.h")
            for m in re.finditer(r'`((?:Iso15693|NfcMagic)[A-Za-z0-9_]+)`', l):
                if m.group(1) not in src:
                    flag("ident", m.group(1), "not found in any .c/.h")

            for m in re.finditer(r'#(\d{2,4})\b', l):
                n = m.group(1)
                r = subprocess.run(["gh", "api",
                                    "repos/xMasterX/all-the-plugins/issues/" + n, "--jq",
                                    '"\\(if .pull_request then "PR" else "issue" end)/\\(.state)"'],
                                   capture_output=True, text=True)
                j = r.stdout.strip()
                if r.returncode != 0 or not j:
                    flag("issue", "#" + n, "does not exist on the repo")
                elif j.startswith("PR") and re.search(r'issue\s+#' + n, l, re.I):
                    flag("issue", "#" + n, "called an issue, but it is a %s" % j)
        if notes_only:
            print("  (%d hit%s in the preamble only -- those never reach him)"
                  % (notes_only, "" if notes_only == 1 else "s"))
        print()
    print("%d thing%s to look at in text that gets posted." % (bad, "" if bad == 1 else "s"))
    return 1 if bad else 0

sys.exit(main())
