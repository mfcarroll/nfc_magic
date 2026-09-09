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

            for sha in re.findall(r'(?<![0-9a-zA-Z])[0-9a-f]{7,9}(?![0-9a-zA-Z])', l):
                # Do NOT skip all-digit tokens: 7883953 was a real rewritten commit in a real
                # draft, and skipping it is how it survived a sweep. git is the decisive test --
                # a plain number does not resolve as a commit, so there is no false-positive cost.
                if subprocess.run(["git", "cat-file", "-e", sha + "^{commit}"],
                                  capture_output=True).returncode == 0:
                    flag("SHA", sha, "resolves locally -- a DEV sha is never valid in a PR comment")
                else:
                    flag("SHA?", sha, "does not resolve; if meant as a hash it is dead")

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
