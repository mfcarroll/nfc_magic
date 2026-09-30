# Round 15 — the delta review, and what came of it

A fresh session reviewed everything that changed since the pushed build (`846a82eb`, backed up as
`backup-pre-review3-replay` in the fork) against the replayed branch at `390a6621`, per the brief
that was in NEXT-SESSION. Single-threaded, report-only, no agents. It found six things, none
blocking. Each was then checked against the code before anything changed; all six held, two of them
narrower than reported. mfcarroll's calls are below, and every fix went into the sync point that
owns it (fold 10, [fork-messages/README.md](fork-messages/README.md)).

## The findings

**1. A clone that loses its card at the new UID re-read (05).** The write-fail scene's CardLost note
said "a clone has no identity check to skip", false since 05 added Iso15693WriteStateVerifyClone,
and the scene could not tell that CardLost from others, since `clone_uid_recheck` was run control.
Checked: real, and wider than reported. A converted clone lost *mid-pass* is in the same position,
because the repair goes out before the pass's own card-present check; and a card lifted at block 34
still has 56/57 sent their frames, since the pass learns the card has gone only at its end. Retry
recovers the card on every route. **Decision: add the note now.** New result field `uid_recheck`;
a clone's CardLost with it set gets Details and a "UID not re-checked" note, as a card-lost wipe
does. Covers the mid-pass route too.

**2. The uid_unexpected contract (05, 07).** Three comments -- the result field, the Fail event, the
reason -- named only the gen2 backdoor, after 05 added the clone's re-read and 07 the half-written
gen1 UID as routes. Checked: real. **Fixed**, each route as a line of its own at the commit that
adds it, so 05's and 07's additions never rewrite each other's lines or the ones 10 corrects.

**3. A bystander at the new UID checks (07, 05).** 07 took any readback that was neither the
original nor the target as a half-written gen1 UID, which outranks the gen1-failed screen: a second
tag answering the inventory got printed as the card's UID and the "56/57/62/63 may be overwritten"
warning was lost. A regression this round, since before 07 that case got the right screen. At 05 the
same exposure is a second case of the documented #251 limit, since the gen2 verify has it before any
data is written. **Fixed exactly at 07**: only the two UIDs a half can leave, 56's without 57's or
57's without 56's, count as this card's. **At 05, docs only**: the release notes' one-tag bullet, the
reply's #251 paragraph and the squash message's #251 bullet (which the review missed) now name the
clone's re-read beside the wipe's.

**4. The progress figure on a converted run (05).** `done++` counted the write that moved the UID,
which the conversion then deducted from the total, so a pass the clock cut at 62/63 showed 61 / 60.
Checked: real, smaller than stated. The step cannot pass 8 (a converted run's total is at least 56,
and `done` is at most two over), so the event bound holds. **Fixed** with `done--`. The special case
for a finished pass stays, despite the review's suggestion: a run whose three attempts at 56 were all
lost converts at 57 having counted 56, and would still read one over.

**5. A taken-back failure still marking later writes (05).** A failure at 56 recorded before 57
moved the UID set `wrote_above_failure` for every later success, which the conversion's take-back
did not undo, so a real capacity edge above became Partial. Checked: real; it needs no contrived
file, only all three attempts at 56 lost to a transient. **Fixed**: once the run has converted, a
success does not count failures the conversion takes back. Only a gen1 card converts, so gen2 runs
are untouched.

**6. "Geometry" after S5 (06).** The block-count flag compares the count only. Checked: cosmetic,
since block count is part of the geometry; broader rather than false. **Fixed** in the four places
that name what is compared: two headers, the compare function's own comment, the release notes.

**The "162 lines" doubt.** Settled: it is the entry's text from its first line to its last, without
the heading and the blank lines around it. After the fold it is 163 lines (of 344), and the reply
says so.

## Verification

**Benched by mfcarroll, 2026-09-29**, on the fold-10 build: a clone lifted near its end shows "Write
failed / Card removed before the write could finish", and its Details reads "Clone notes -- UID not
re-checked: the clone sent writes to blocks 56/57...". "Looks good to me."

Each code fix has a host test, and each test was seen to fail with its fix reverted (eight
mutations, each caught by its own test; 205 tests at the tip). The fold's first attempt failed its
per-commit checks at 05-06: 05's notes page joins notes with a plain newline, since `begin_note`
arrives at `6038831`, and before 07 the fake does not move the UID on an absent 56/57. The fold was
redone from the original history with the note in two forms and fixtures both fakes honour. Its
test replay then measured the push's churn at 21 lines, 15 of them the twin comments rewritten at
05, again at 07 and again at 10; a third pass wrote each route as a whole added line instead, which
put churn back at 6.
