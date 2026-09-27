#!/usr/bin/env python3
"""Find dev SHAs in the notes that no longer name a commit anyone can reach.

    tools/check-stale-shas.py                 the live files (default)
    tools/check-stale-shas.py --all           every tracked .md under .notes/

WHY. This round rebuilt its history five times, and every rebuild orphans the SHAs the notes were
citing. A note that says "shipped as b312653" then points at nothing: `git show` fails, and the next
session either chases it or quietly trusts a stale sentence. The fork messages already ban dev SHAs
outright for the same reason; the notes cannot, because they are how we find our own work.

WHAT COUNTS AS STALE, and the distinction matters:
  - reachable from ANY ref (a branch, a tag, a safety branch) -- FINE. `wip-pre-repair-reorder`'s
    tip is not an ancestor of HEAD and is not supposed to be.
  - a real commit object but reachable from nothing -- STALE, and a `gc` will take it.
  - not an object at all -- ignored. Most seven-hex words in prose are not SHAs.

ARCHIVED ROUNDS ARE EXEMPT BY DEFAULT. pr-round-5 through pr-round-14 describe history as it stood,
and rewriting their SHAs would falsify the record rather than fix it. Only the live files are
checked: the state file, the current round, and the rule files.

AND NEXT-SESSION.md HAS AN ARCHIVE INSIDE IT. Everything from its first "Where things stood before"
heading down is the same kind of record -- old rounds, written when their SHAs resolved. Only the
head of that file is checked. If a stale SHA matters below the line, it is because someone is
reading history, and history is what it is.
"""
import argparse, re, subprocess, sys

LIVE = ("NEXT-SESSION.md", "WRITING-RULES.md", "BENCH-RULES.md", "REVIEW-PROMPT.md",
        "capability-matrix.md", "firmware-gaps.md", "iso15693-primer.md",
        "gen3-candidate-slix2-gold.md", "gen1-hardware-findings.md")
LIVE_DIRS = ("pr-round-15/",)


def sh(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True).stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true", help="every tracked .md under .notes/")
    a = ap.parse_args()

    files = sh("git ls-files '.notes/*.md' '.notes/**/*.md'").split()
    if not a.all:
        files = [f for f in files
                 if f.split("/")[-1] in LIVE or any(d in f for d in LIVE_DIRS)]

    # Everything any ref can reach. Built once: --contains per SHA is far too slow.
    reachable = set(sh("git rev-list --all").split())

    stale = {}
    for f in files:
        lines = open(f, errors="replace").read().split("\n")
        if f.endswith("NEXT-SESSION.md"):
            cut = next((n for n, l in enumerate(lines)
                        if l.startswith("## Where things stood") or l.startswith("## Where things stand")), len(lines))
            lines = lines[:cut]
        for i, line in enumerate(lines, 1):
            for m in re.finditer(r"\b([0-9a-f]{7,40})\b", line):
                sha = m.group(1)
                if re.fullmatch(r"[0-9]+", sha):
                    continue
                full = sh(f"git rev-parse --verify -q {sha}^{{commit}} 2>/dev/null").strip()
                if not full or full in reachable:
                    continue
                stale.setdefault((sha, full), []).append((f, i))

    if not stale:
        print("0 stale SHAs in %d file(s)." % len(files))
        return 0
    print("%d SHA(s) in %d file(s) name a commit no ref can reach:\n" % (len(stale), len(files)))
    for (sha, full), where in sorted(stale.items(), key=lambda kv: -len(kv[1])):
        subj = sh("git log -1 --format=%%s %s" % full).strip()
        print("  %-10s x%-3d %s" % (sha, len(where), subj[:62]))
        for f, i in where:
            print("        %s:%d" % (f, i))
    print("\nCite the SUBJECT, or re-derive. A SHA in a note is a claim that goes stale silently.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
