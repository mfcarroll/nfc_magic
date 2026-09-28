# The whole-PR prose review — run this in a fresh session

Written 2026-09-28, after two review passes over round 15. Every round so far has reviewed its own
DELTA. That is why the defects below kept turning up in text written rounds earlier: nobody had read
that text since. **This review reads everything the PR ships, once, against one checklist**, and
reports. It fixes nothing.

## Why now

Round 15's two review passes found these in text older than the round:

- a stale cross-reference — "Same root cause as the unaddressed writes above", after the writes were
  addressed
- a wrong count — "two of the three chips … are under the boundary", when all three are
- speculation stated as function — "UID/unlock/commit" at five sites
- the bench leaking into shipped text — "not observed here"
- title comments left stale when the title logic changed
- history in a comment — "the screens no longer promise"

The PR adds about **4,900 lines of C, 2,000 of them comment-bearing, plus 158 lines of release
notes**, across 27 files. That surface has never been read as a whole.

## Scope

**IN.** Every shipped file the PR adds or changes against upstream. In the fork:

    git -C ../all-the-plugins diff --stat $(git -C ../all-the-plugins merge-base HEAD upstream/dev) HEAD -- base_pack/nfc_magic

Review the dev repo's copy of those files (same content; the replay verifies it). That is:

- `magic/protocols/iso15693/*`
- every `scenes/nfc_magic_scene_iso15693_*.c`
- the ISO15693 parts of the shared files the PR touched: `nfc_magic_app_i.h`, the write, confirm,
  file-select, magic-info and partial-details scenes, and the scanner
- CHANGELOG 2.3
- **every user-facing string** in those files

**OUT:** upstream code the PR did not change; `tools/` and `.notes/`, which never ship; and the
host-test harness, which is its own PR — though a harness comment that contradicts shipped code is
worth a line.

**Code behaviour is out of scope EXCEPT where a comment and the code disagree.** Then report both
sides and say which is right, with evidence. Do not propose code changes as part of this review.

## What to look for

Each class below shipped at least once. The test for each is the question in bold. Use
`.notes/REVIEW-PROMPT.md`'s eighteen classes and `.notes/WRITING-RULES.md` as the base; these are
the ones that bit hardest.

1. **Is a claim corrected here still standing elsewhere?** Stale twins. For every claim that moved in
   the round's history, grep its exact words across all shipped text.
2. **Does the code do what the comment says?** For example, `card_blocks` "is 0 while neither flag
   is set" was false, and the size note depended on it being false.
3. **Is a measurement stated as a law?** Five cards of unknown history do not make "those registers
   take a write with nothing in front of them" a property of gen1. State the sample, then the
   conclusion the reader needs.
4. **Is an inference stated as fact?** What 62/63 do. The arm model. Anything proxmark does without
   explaining why.
5. **Does every "every", "only", "all", "never" and "no" survive the code?** "Every other frame
   carries a UID" was false for reads, inventories and GET SYSTEM INFO.
6. **Is a count true only of most cases?** "Eight reads" held only when the card ends where the source
   does. State the property instead.
7. **Does shipped text know where the bench is?** "here" meaning the test bench, card nicknames from
   `tools/tag-inventory.json`, "this shelf". A reader of the code has no bench. "Here" meaning "in
   this code" is fine.
8. **Is the comment telling the code's history?** "now", "no longer", "was right when", "for as long
   as", "in the first place", "used to" — where they describe the code's past rather than runtime
   order. The reader is deciding whether they may change the line in front of them.
9. **Does a comment keep a list of what OTHER code does or does not do?** That list is a duplicate by
   construction. Each site should state what it does itself.
10. **Is the same fact stated in more than one place?** Name the home; the others point at it. A
    pointer must name its target and must not point at another pointer.
11. **Does a sentence carry a constraint, or just a flourish?** "a claim is a costume", "a guess
    wearing the clothes of a measurement".
12. **Release notes: is this the end state, for a user?** Scoped, true of every path — not "a plain
    success" where a survey note can appear — and consistent with the code and the screens.
13. **User-facing strings: are they accurate and consistent with the notes?** No internal vocabulary,
    no speculative names.
14. **Chip or family? Cards or chips?** Name the chip; count cards and chips separately; never count an
    unidentified card as a chip.
15. **Mechanical:** blocks over 20 lines (`tools/check-writing.py comments` warns), lines past the
    file's width, lines broken early, two comments run together without a `//` break.

## Method

**Single-threaded by default.** mfcarroll's rule is to ask before any fan-out. If time matters,
propose a split first, with its cost: for example the poller in two halves, the scenes, then the header
plus release notes and strings, with one verification pass. Then wait for a yes.

1. **Mechanical first, and cheap.** Run the gates:

       tools/check-writing.py comments <every shipped .c/.h>
       tools/check-writing.py headings .notes/pr-round-15/reply.md

   Then grep each of classes 4, 5, 7 and 8 across the scope; `git grep -P` if `\b` matters. **Check a
   gate actually read its input** — `forkmsg` once reported a clean scan of nothing.
2. **Read every comment block in scope, in file order.** For each claim, find the code it describes
   and check it, and find the measurement record behind any measured claim (`.notes/pr-round-*/`,
   `.notes/gen1-hardware-findings.md`, `tools/tag-inventory.json`). The poller alone is about 2,400
   lines with a comment on nearly every other line, so plan two sittings if needed.
3. **Cross-file last.** For each fact found in more than one place, pick its home.

**Re-derive everything at the moment of reading.** A number, a chip count or a SHA in a note is a claim
too.

## Output

`.notes/pr-round-15/app-review.md`, findings only. Round 15 is not posted yet, and a finding in its
own text may fold into it before the push. For each finding:

- the site (`file:line` at the reviewed SHA — record it at the top)
- the class
- the text, quoted
- the evidence: the code line, the measurement record, or the contradicting site
- proposed text
- whether the line is from round 15 or earlier, since that decides whether a fix folds into round 15's
  commits or goes in a new one

End with a table of findings by class and by file, and a short list of the ones that matter most.

## Constraints

- **No edits** to shipped files, and no commits except the findings file (dev-only).
- **No pushes and no posts.** Round 15 is built in the fork and unpushed; do not touch it.
- Do not reopen anything under "WHAT TONIGHT SETTLED" in `.notes/NEXT-SESSION.md`, and **do not wipe
  `slix2-gold-30mm`**.
- `[👤]` paragraphs in any draft are mfcarroll's own words; leave them alone.

## After it

The fixes are a fold, like review 2's (`.notes/NEXT-SESSION.md`, the review-2 and follow-up
safety-branch sections, has the method: a scratch series, an index-filter, every sync point tested).
**How much ships in this PR is mfcarroll's call.** Weigh it against the maintainer's diff: accuracy
defects in shipped text are worth a round; simplification alone may be better as a follow-up after
merge.
