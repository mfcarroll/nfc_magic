# Next session — ROUND 15: THE DELTA REVIEW IS FOLDED IN (fold 10). A fresh replay comes next, then the push.

**⚠️ CORRECTED 2026-09-26: force-pushed `dbc11980` -> `1d411dec`**, our seven rebuilt as intended plus
an eighth, 8 of 8 signed, lease on `dbc11980`; posted as
[issuecomment-5852338281](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5852338281).
See the section below. **Originally pushed `749f10e6..dbc11980`, a fast-forward of 7, all signed.** PR head confirmed via the API
before posting. **Posted:** the main reply as
[issuecomment-5824709850](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5824709850),
and all seven thread replies, each verified by `in_reply_to` AND by the root's file.

**He took the branch himself this round** -- `a27d187d..749f10e6`, twelve commits, four behavioural.
All twelve adopted into dev; three of our round-13 commits were moot because he reached the same
conclusions independently. We added seven on top.

**THE BENCH IS DONE.** His `0c5a7d69`, `62b60e2b` and `4d20f06e` all pass on hardware; `749f10e6` was
settled against the firmware source. Our own `dbc11980` consent fix is confirmed end to end by the
two-card Retry.

**gen-2-card geometry is RESOLVED (2026-09-26):** re-cloned/wiped to 64 physical blocks, all-zero UID,
blank. It had been advertising 256 (then 28 on a SLIX fixture) from earlier CFG/fixture runs. Recorded
under `reset_2026_09_26` in `tools/tag-inventory.json`.

## NEXT: A FRESH REPLAY OF THE THIRTEEN, THEN THE PUSH — each step on mfcarroll's go-ahead

**The delta review is done and folded in** ([pr-round-15/delta-review.md](pr-round-15/delta-review.md)).
A fresh session read everything since the pushed build and found six things, none blocking; all six
held when checked against the code. mfcarroll's calls, 2026-09-29: the clone's "UID not re-checked"
note goes in now (finding 1), and 2-6 are fixed, the 05 half of 3 in the docs only. Fold 10 put each
fix at the sync point that owns it -- 05, 06, 07 and 09 -- and the fork-messages README's fold-10 note
lists them. Messages 05, 06, 07 and 09 say what they now carry; the reply, the squash message and the
2.3 count (163 lines now) are updated. Safety branch `wip-pre-fold10` = `7238806`. Two earlier attempts
were dropped; [delta-review.md](pr-round-15/delta-review.md) says what each got wrong.

**Verified:** 205 host tests at the tip, and eight mutations, each caught by its own test; 246 pairs,
2260 mapped blobs, 0 problems; the per-commit check on all 29 rewritten commits that touch shipped
files or tests, 29 of 29 clean (markers, host tests, 71 units under -Werror); the comment gate, the
message gate and the drafts check, 0 findings each; a test replay into a `--shared` clone pinned at
`1d411dec`, 13 commits, its tree equal to a full sync of the last anchor and a fast-forward of the
pushed base; churn 6 lines, each in the README's residual section; clean FAPs on Momentum and
Unleashed, 71 units each. **Benched** by mfcarroll: a clone lifted near its end gets the lost-card
screen with Details, and the new note reads as intended.

**Still to do, in order:**

1. The real replay, which needs 1Password unlocked. It replaces the fork's `390a6621`, built before
   fold 10, never pushed, and kept as `backup-pre-fold10-replay`.
2. Then, each on its own go-ahead: the PR push, a fast-forward of 13 on `1d411dec`; the #255 title and
   body edit and comment, before the #250 reply; the dev force-push of iso15693-dev, with a lease; and
   the squash message at merge.

## THE THIRTEEN SYNC POINTS, AND FOLDS 7-9. Safety branches `wip-pre-fold10`, `wip-pre-fold9`, `wip-pre-fold8`, `wip-pre-fold7`, `wip-pre-review3-fold`

**State, 2026-09-29 night.** Thirteen sync points. mfcarroll's bench
([pr-round-15/review3-bench.md](pr-round-15/review3-bench.md), reading at its end) decided S5 (fold 8:
the geometry note compares the block count only, with a mutation-checked host test). His calls after
it: **fix SLIX saves this round** (fold 9) and **record "Cloned 28/64" as future work** (below). Dev
has DIVERGED from origin/iso15693-dev; force-pushing it needs his go-ahead. Nothing pushed or posted.

| # | anchor | # | anchor | # | anchor |
|---|---|---|---|---|---|
| 01 | `d09f499` | 06 | `d44fe7b` | 11 | `0b8e7ef` |
| 02 | `4a3e382` | 07 | `15733bf` | 12 | `ade66cd` |
| 03 | `e7663ff` | 08 | `764ce9f` | 13 | `eced38d` |
| 04 | `53faffb` | 09 | `7a74e86` | | |
| 05 | `2a8de82` | 10 | `d0510a7` | | |

**Fold 9** (`$SP/fold9/rules9.py`; the engine gained an `add` rule for a new file, and the index filter
`--add`): `676842c`, notes-only and directly on 07's anchor, became sync point 08 carrying the
one-condition file-select change (`nfc_protocol_has_parent`, exported in both SDKs) and its host test
`test_file_select_scene.c` (mutation-checked: without the new term the SLIX case fails; it completes the
MIFARE types the scene reads itself, leaving the shared fakes alone). Its dev message was rewritten to
say what it now carries. The release-notes sentence is in 09, from `82f9a84`. 127 pairs, 622 mapped
blobs, 0 problems. 07's message no longer says the notes are "in the next commit"; 09's WHAT IS NEW
names SLIX saves; README, reply and squash message renumbered and updated.

**Superseded by fold 10:** the anchors in the table are fold 10's; the replay described next is of
the build before it, and a fresh one is step 1 above.

**Benched and replayed, 2026-09-29 night.** S1 and S2 passed (his stock-app SLIX save back onto its
tag, proxmark readback matching; and onto gen-2-card); 08's message carries the S1 measurement. The FAP
built clean on both firmwares and the device copy was verified. **The real replay is done: the fork's
`nfc-magic-iso15693` is at `390a6621`, 13 commits on `1d411dec`, 13 of 13 signed, every tree identical
to the test replay's**, the old build backed up as `backup-pre-review3-replay` = `846a82eb`.

**Waiting on mfcarroll's go-ahead, each separately:** the PR push (a fast-forward), then the #255
title/body edit and comment, then the #250 reply; the dev force-push of iso15693-dev; and at merge the
squash message.

**FUTURE WORK, recorded by mfcarroll's call:** "Cloned 28/60" for a 64-block file through gen1 should
read against the file's count, "Cloned 28/64 / Not written: 36" -- his case: "I tried to write a 64
block file, why is it only saying 60". Not this round: `blocks_total` deducts the four registers since
`20649a5`, and three places decide outcomes from it -- the Partial screen's count, the poller's
all-rejected guard (Fail vs Partial) and the write scene's reason routing (Clone failed vs Not a magic
tag) -- so all three must count the skipped registers, the result needs the count rather than a flag,
the conversion in 05 stops deducting, and the rationale in 03, 05 and 09's comments and messages
changes. A follow-up of its own, where the counting contract can be reviewed in isolation.

**Filed separately, by his call:** the tone replaying on Back from Details (upstream's pattern).
**Optional:** "The card has UID only." understates a gen2 card (text from `6f85395`).

[pr-round-15/final-review-3.md](pr-round-15/final-review-3.md) found two behaviour bugs, about ten
false claims in shipped text, and a set of draft errors. **mfcarroll's calls, 2026-09-29: all of it
goes in THIS round**, to get as close to merge as possible without needing another review.

- **The gen1 Partial becomes a question about data** (the Decided section there). A gen1 clone whose
  file reaches 56/57/62/63 with nothing there ends as "Clone finished" with a note, not Partial.
  - 03 takes the gate, and at 03 that case gets the plain success screen.
  - 06 adds the note. The Details line stays; the summary line needs its own entry.
  - 08 updates the notes.
- **A:** a verify STATE after a gen2-path pass that reached 56, entered by NfcCommandReset. That is
  the file's own rule at poller.c:312, and not an inline inventory. A mismatch reports UidUnexpected;
  no answer reports CardLost.
- **B and D:** fix.
- **E:** both halves. A presence check when the survey stops, and the 100% frame moved after the
  survey.
- **S5:** drop the size term ONLY if the bench shows no card reaches the note on size alone;
  otherwise print the size.
  - UNTESTED so far.
  - Fixture: `tools/test_nfc/iso15693_blocksize8_28.nfc` (`5a348e6`).
  - Runs: a gen1 SLIX through the opt-in, and gen-2-card, on the current build.
- **Text and messages:** every Important and Suggestion fix that holds, all re-checked 2026-09-29.
  poller.c:279 goes back to the maintainer's round-6 wording.
- **Simplifications:**
  - S2, S3 and S6-S10 fold where they belong.
  - S11 and S12 become a new closing commit, **13**, for earlier-round code. (That premise was wrong
    in part -- see the state above.)
  - S4 is left.
- **Counts that would change twice** drop the count in favour of per-field notes, so there is no
  churn: T6 "three exceptions", T8 "four callers".
- **The reply is rewritten shorter** (R9): disposition, correction and finding, pointing at the
  commit messages for the arguments. R1-R8 are fixed in it.
- **#255:** edit its title and body to the current state, with a dated "Updated" note, and post a
  short comment saying what changed, plus the gen3-a result. Both on go-ahead.
- **Verification as usual:**
  - host tests for every new branch, mutation-checked
  - every sync point: markers, tests, 71 units
  - FAP, clang-format, gates, churn
  - an in-context read of the whole fold diff against `846a82eb`
  - the test replay, then the real one
  - mfcarroll's bench

**Order:**
1. Code at the tip, with its tests.
2. Map each change to its owner, with the engine as in earlier folds.
3. Fold.
4. Verify.
5. Messages.
6. Replay.
7. Drafts.
8. Bench.
9. Push, on go-ahead.

## ⚠️ SAFETY BRANCH FOR THE HARDENING AND J1 — `wip-pre-j1-fold` = `cd23a47`

mfcarroll's call, 2026-09-29: "go ahead with 1-4, leave y3". The round-10 hardening moved INTO THIS
PR rather than after merge -- this is the component's first release -- as two new sync points, and
J1 folded into the commits that wrote its lines:

- **11** (`ea40479`): Y1 + X1. mark_failed/unmark_failed furi_check the index against
  ISO15693_POLLER_MAX_BLOCKS, as the SDK's own block accessor checks its index; the two
  iso15693_3_get_block_data calls cast their uint16_t block number, as the third already did.
- **12** (`26da484`): Y2. Both write-fail reason switches switch on the enum with every reason
  listed and no default, so -Wswitch under the firmware's -Werror catches a missing one, then
  furi_crash -- the shape mishamyte's own commit gave the write-state switch. The enum opens with
  Unset = 0, so a screen entered without a reason crashes instead of saying "Not a magic tag". Every
  entry into the scene sets its reason on the line before it navigates, so nothing reachable does.
- **T3**, dev-only: the Fail ladder's eight branches in order, other protocols' shared screen, the
  gen1 consent screen's buttons and bodies, the Unset crash (a forked child, `aborts.h`), the
  bitmap bound. Every new test was mutation-checked. **Y3 is left, by mfcarroll's call.**
- **J1**: the TI chip is named by what identified it. "(by its behaviour, not its UID)" in the
  poller's chip list (01), in fork message 01's copy of it, and in the Validation bullet (08's
  range); the OPTION release note says the one measured was "identified by this behaviour rather than
  by its UID" (08's anchor). The name comes from the type line both cards' first UIDs decoded to;
  what confirms it is the 0x03 OPTION refusal, which `black-tag` gave while WEARING AN NXP UID. Both
  are gen2 magic, so their UID, IC ref and geometry are all settings. Later mentions (02's comments
  and message, 04's AFI comment, the squash message, the reply) stay as names: each says what the
  chip does, not how it was identified.
- **The latch passage at the data-block re-address STAYS -- it is not a duplicate.** It records a
  write to block 56 ALONE, re-read in the same field session, on all three gen1 chips -- the fact the
  re-address after a single UID-block write depends on. The home at ISO15693_MAGIC_BLK_UNLOCK records
  the four-frame sequence and then an inventory. They overlap in four words, "with no power-cycle".
- **The 13 long comment blocks were read against the maintainer's received comments: none cut.**
  No history and no repetition; every past tense in them is card state or a measurement. The
  maintainer asked to keep, or endorsed, the write scene's Back-swallowing block ("worth keeping the
  rest of this comment as it stands", round 6), the 36-of-64 bench (round 2), the tail-drop's
  prefix/pvPortMalloc reconciliation (rounds 5-6), PASS_MAX_MS's "defensible for the more expensive of
  the two" (round 6), and the wipe verify's positive-observation rule (round 3). J1 makes the
  addressing block 21 lines -- any wording does, even fully rewrapped -- so the gate warns 14 times.

**How.** One `filter-branch --index-filter` pass, three exact rules applied from their owners
(`e5227c1`, `6ca0d62`, `68659cb`, pre-fold names) to HEAD, each matching exactly once in every
commit: 232 commits paired with identical authors, dates and messages, 332 mapped blobs and nothing
else changed. Every SHA from 01 on is new.

**Verified 2026-09-29:** all 25 shipped commits -- no markers, host tests pass, 71 units under the
firmware's -Werror; clean FAP build, 71 CC and APPCHK; clang-format clean on 95 files; 180 host tests
at the tip; churn still 2 lines; every writing gate at 0 findings. A test replay into a `--shared`
clone made 12 fork commits, its tree identical to a full sync of 12, a fast-forward from `1d411dec`;
its range-diff against `feed76cf` shows only J1, the message edits and context shifts.

**Fork messages:** 11 and 12 written fresh from the diffs; 01, 04 and 07 rewrapped so no line opens
with "--" and 04's table line fits 80.

## ⚠️ SAFETY BRANCH FOR THE FOLLOW-UP FOLD — `wip-pre-followup-fold`

Created 2026-09-28, on mfcarroll's go-ahead, before folding his follow-up to review 2 into the round-15
commits:

- **naming:** UNLOCK/COMMIT disclaimed once at their defines; shipped text stops stating what 62/63
  do, and the details screen calls them the "UID / backdoor registers"
- **scope:** "here" meaning the bench becomes "tested" in comments and release notes
- **accuracy:** "plain success" qualified where the survey notes can contradict it
- **06's comments:** reduced to one home per fact

The name is authoritative and includes the commit recording it. Nothing at or below `8b8f310^` moves.
The invariant is the same as the review-2 fold's, below: the shipped diff must be only these changes,
and `.notes`/`tools` must be EMPTY until notes are edited.

**DONE AND VERIFIED 2026-09-28**, by the same method as the review-2 fold:

- the tip diff is byte-identical to the reviewed scratch result
- 189 commits, messages byte-identical
- all 18 shipped commits: no markers, own tests pass, 71 units under `-Werror`
- churn still 2 lines
- 01 and 02 keep their SHAs; everything from 03 on is new

**On top, dev-only:** `tools: pin the gen1 sequence frame by frame`. The fake logs every addressed
write, and one test pins the four gen1 frames in order. It kills the dropped-unlock mutant that
survived review 2, plus four more. 169 tests.

The whole-PR prose review mfcarroll asked about is scoped in [APP-REVIEW-PROMPT.md](APP-REVIEW-PROMPT.md),
for a fresh session.

## ⚠️ SAFETY BRANCH FOR THE REVIEW-2 FOLD — `wip-pre-review2-fold`

Created 2026-09-27, on mfcarroll's go-ahead, before folding
[pr-round-15/final-review-2.md](pr-round-15/final-review-2.md) into the round-15 commits. The fold
covers:

- every comment and release-note fix in that file
- both code simplifications
- the #251 paragraph, dropped
- churn cut from 69 lines to 2, by writing each line in its final form at the first sync point
  that has it

The name is authoritative and includes the commit recording it. Nothing at or below `8b8f310^` moves,
so round 14 and the PR head `1d411dec` stay as they are.

    git diff wip-pre-review2-fold HEAD -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h
    git diff wip-pre-review2-fold HEAD -- .notes tools

- **The first** must be ONLY review 2's changes: comment and release-note text, plus the two
  simplifications in `iso15693_poller.c`.
- **The second** must show ONLY `tools/hosttest/test_addressed_write.c`, whose frame test calls
  `write_block_addressed` now that `send_gen1_frame` is gone. Notes edits come after that.
  `test_write_fail_scene.c` changes too, but only at intermediate commits (the 03 range), never at
  the tip.

If either check fails, `git reset --hard wip-pre-review2-fold`.

**DONE AND VERIFIED 2026-09-27.** Method:

- **The rewrite.** An `--index-filter` replayed precomputed blobs: every shipped commit's seven files,
  as built in a scratch series that re-applied the 19 shipped changes by hand, with each conflict
  resolved and checked for markers.
- **186 commits**, messages byte-identical, and nothing at or below `8b8f310^` moved.
- **The tip diff** is byte-identical to the reviewed scratch result.
- **All 18 shipped commits**: no markers, their own host tests pass (132 to 168), and all 71 units
  compile under `-Werror` with the firmware's own compile commands.
- **The tip**: the FAP builds from a clean object dir (71 CC, zero app warnings, APPCHK), and 95 files
  are clang-format clean.
- **Churn is 2 lines**, down from 69.

**18 shipped commits now, not 19.** `8c5096a`'s shipped change (the seven-card measurement) moved
to 01 and 02, so its rewrite touches notes only. **Every SHA from `8b8f310` onward is new**; the
`.msg` filenames and the fork README table are the anchors.

**Three test fixes rode along, dev-only**, each the test half of a shipped change that moved:

- 01's frame test passes the flags byte (from 02)
- 03 to 07's gen1-caveat tests assert the final wording (from 06)
- the gen1 frame test calls `write_block_addressed` (07 onward)

**Mutants on the two simplifications**: five of six killed. The dropped-unlock mutant survives, and
it survives on the old code too -- a gap in the harness, not something the fold opened.

## ⚠️ SAFETY BRANCH FOR THE ROUND-15 FIXES — `wip-pre-round15-fixes`

Created before folding the final review's shipped-text fixes into the round-15 commits that wrote
them: the block-57 boundary (six sites), the addressing define's seven-card read-back, the repair's
"ARMS", the gen2 builder's comparison, and the 2.3 release notes. The name is authoritative and
includes the commit recording it. Round 14's eight commits sit below the rewrite and do not move.

    git diff wip-pre-round15-fixes HEAD -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h
    git diff wip-pre-round15-fixes HEAD -- .notes tools

The first must be ONLY those fixes, the .c/.h part comment-only; the second EMPTY until notes are
edited. Otherwise `git reset --hard wip-pre-round15-fixes`.

## ⚠️ ROUND 14 WAS PUSHED FROM ORPHANED ANCHORS, AND IS BEING CORRECTED — `wip-pre-round14-correction`

Found by the final review, [pr-round-15/final-review.md](pr-round-15/final-review.md) F1. The PR holds
dev `868c598`'s tree, which no branch contained; `wip-round14-as-pushed` keeps it. mfcarroll's call
2026-09-26: force-push a corrected round 14 FIRST, then do round 15 -- the longer the wrong content
stands, the likelier mishamyte starts work on it.

**The safety branch `wip-pre-round14-correction`** -- the name is authoritative, and it includes the
commit recording it -- was created before folding five comment fixes into the commits that wrote them:
the clock cut, the clamp, and `0c76458` (`f899c78` after it), which becomes the eighth commit of
round 14. The rewrite is tree-level and touches two files. **Check it before any note is edited:**

    git diff wip-pre-round14-correction HEAD -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h
    git diff wip-pre-round14-correction HEAD -- .notes tools

The first must show ONLY the five comment fixes, in `iso15693_poller.c` and `nfc_magic_scene_write.c`;
the second must be EMPTY. Otherwise `git reset --hard wip-pre-round14-correction`. Every later SHA
moves, so every round-15 anchor is renamed after it.

**DONE, and verified:** the rewrite (5 hunks in those 2 files, proven comment-only; `.notes/` and
`tools/` untouched; 200 commits, messages byte-identical); the correction set in
[pr-round-15/round14-correction/](pr-round-15/round14-correction/), eight anchors, each checked for
markers, its own tests, `-Werror` compile and imports; round 15's eight `.msg` files renamed, and
both anchor tables re-derived by subject. `replay-to-fork.sh` now refuses an anchor that is not on
the branch and a fork base that is not dev's tree before the first anchor -- the two checks that
would have caught this -- and gates every sync point's tree instead of HEAD's.

**BUILT, signed, 8 of 8, in the real fork: head `1d411dec`** -- trees and messages identical to the
throwaway build that was tested, the stale round-15 replay kept as `backup-stale-round15-replay`.
**PUSHED AND POSTED 2026-09-26, on mfcarroll's go-ahead.** `git push --force-with-lease=
nfc-magic-iso15693:dbc11980...` took the branch `dbc11980` -> `1d411dec`; the PR head reads `1d411dec`
through the API, 144 commits, and the timeline records the force-push. The comment went up as
[issuecomment-5852338281](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5852338281),
its body checked against the payload. **Its compare link uses TWO dots** -- with three, GitHub diffs
from the common ancestor, which after a force-push is his head, and showed all of round 14, code
included, under a sentence saying comment-only. Caught before posting by opening the link.

**Round 15 now builds on `1d411dec`**, and its replay's base check holds it there.

## ✅ SETTLED — THE GEN2 BACKDOOR CANNOT BE ADDRESSED, MEASURED ON FOUR CARDS

mfcarroll asked whether leaving it unaddressed was an oversight. **It is not a choice at all.**
Measured 2026-09-26 on `gen-2-card`, `black-tag`, `white-coin` and `v2-sticker-50x28`: each takes
the unaddressed `02 E0 09` and refuses the addressed form -- correct UID, with and without OPTION,
and with the address bit set but no UID in the frame. All four also take an addressed ordinary
WRITE BLOCK and go silent on a wrong address, so the frames are well formed and the cards' addressing
works. The backdoor is not reachable that way. Transcripts in
[pr-round-15/gen2-addressed-bench.md](pr-round-15/gen2-addressed-bench.md).

**Shipped as a REASON change, not a behaviour change**, folded into sync point 08: the measurement
now sits at `iso15693_poller_build_gen2_frame`, the flags define keeps only why the byte is used
literally, and the release note states the residual instead of arguing it away -- another gen2 magic
card in the field takes these frames and nothing here can stop it.

**AND OPTION SWALLOWS THE ACKNOWLEDGEMENT ON `0xE0` TOO**, on all four, including two that do not
want the flag for ordinary writes. The app never meets it -- `ISO15693_MAGIC_FLAGS` appears exactly
once, inside the gen2 builder, while the sticky flag lives in `iso15693_poller_write_flags()` -- so
the finding confirms the hardcoded `0x02` rather than changing anything.

## The argument that sent the gen2 question to the bench, kept because the shipped text rests on it

**It is NOT addressed.** `iso15693_poller_build_gen2_frame` sends `02 E0 09 <ref> d0 d1 d2 d3` --
flags `0x02`, unaddressed. The reason given at `ISO15693_MAGIC_FLAGS`, again at WHAT REMAINS
UNADDRESSED, and in the 2.3 notes is that `0xE0` is proprietary, so "a conforming tag rejects it on
the command and there is no standard frame for a bystander to take."

**That argument covers CONFORMING tags only, and this app exists for the others.** Another gen2
ISO15693 magic card in the field parses `0xE0 0x09` exactly as the target does. It would take the
UID write AND the CFG frame, which reprograms block count, block size and IC reference -- a heavier
payload than gen1's four blocks, on the one population likely to be carrying two magic cards.

**Whether it CAN be addressed is untested, not impossible.** ISO15693-3 makes ADDRESSED a
request-level flag and the conforming placement would be `22 E0 <uid x8> 09 <ref> d0..d3`, but a
backdoor is not a conforming implementation and only the silicon says whether its parser honours the
flag. **The test is two frames on `gen-2-card`** -- addressed with the right UID, then one byte
wrong -- and it is reversible by re-cloning.

**This is the same shape the round already reversed once**: the gen1 sequence carried a comment
saying addressing it was unmeasured, mfcarroll challenged it, and the evidence was already there.
Here the claim is stronger -- "cannot usefully be otherwise". Defensible: the risk is narrower,
because only another gen2 magic card can act on them. Not defensible as it stands.

**mfcarroll's call 2026-09-26 was to fix it after the review, expecting a behavioural change.** The
bench made it a reason change instead -- see the settled section above. The prediction that it would
need its own sync point and a bench was half right: it needed the bench.

## ⚠️ SAFETY BRANCH FOR THE GEN2 WRITE-UP — `wip-pre-gen2-writeup`

Created 2026-09-26 before folding the gen2 measurement into sync point 08. Same invariant:

    git diff wip-pre-gen2-writeup HEAD -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h

**Only `CHANGELOG.md` and `iso15693_poller.c` may differ**, and only in comments and release notes.
Verified after the fold, and every sync point re-checked for conflict markers, compiled and tested.

⚠️ **08's ANCHOR MOVED THREE TIMES TONIGHT** -- the content fold, then the message rewrite, then
moving the comment to the frame builder -- and the `.msg` filename was renamed after each.
**Rewrite the message BEFORE naming the file**, or the rename happens twice for no reason.

## ⚠️ SAFETY BRANCH FOR THE REVIEW FOLD — `wip-pre-review-fixes`

Created 2026-09-26 before folding two review corrections into the commits that wrote the text.
`5438061^..HEAD` was rewritten with `git filter-branch --tree-filter`, which is tree-level and so
cannot produce the conflict markers a patch-level rebase produced earlier in this round.

    git diff wip-pre-review-fixes HEAD -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h

**Must show ONLY the two fixes** -- the CHANGELOG's OPTION bullet and the poller's unlock tally.
Verified after the fold. **Sync points 07 and 08 have NEW SHAs**; 01-06 sit below the rewrite and
are unchanged. **08's moved three times on 2026-09-26** -- the content fold, the message rewrite,
then relocating the comment -- so do not read an anchor out of this file. **The table in
[pr-round-15/fork-messages/README.md](pr-round-15/fork-messages/README.md) is the one place anchors
are recorded, and `tools/replay-to-fork.sh` reads the `.msg` filenames rather than any prose.**

⚠️ **THE FORK IS STALE.** `nfc-magic-iso15693` still holds the eight commits built from the OLD
anchors. It needs `replay-to-fork.sh` re-run before any push -- which rewrites all eight, so it waits
on mfcarroll.

## ⚠️ SAFETY BRANCH FOR THE RELEASE-NOTES MOVE — `wip-pre-notes-move`

Created 2026-09-26 before stripping CHANGELOG.md from every commit in the round and adding it back
as one commit at the end. Same invariant as the other two rewrites:

    git diff wip-pre-notes-move <new HEAD> -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h

**EMPTY, or `git reset --hard wip-pre-notes-move`.**

## ⚠️ SAFETY BRANCH FOR THE CHURN PASS — `wip-pre-churn-pass`

Created 2026-09-26 before rewriting intermediate commits to remove 110 lines of intra-push churn.
**The check is the same one the repair reorder used** and is robust to this file moving:

    git diff wip-pre-churn-pass <new HEAD> -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h

**That must be EMPTY.** Only the intermediate trees change. If it is not:
`git reset --hard wip-pre-churn-pass`.

## ⚠️ SAFETY BRANCH FOR THE REPAIR REORDER — `wip-pre-repair-reorder` = `1c85d7428d5d20480f14ea7088a01023966018bb`

Created 2026-09-26 before moving `628eede` (the gen1-card repair) ahead of `bbf3b77` (the clone
survey), so the repair can be its own fork sync point instead of being buried inside a commit titled
"what a clone leaves behind". The two are ADJACENT in the file, not coupled: the repair references no
survey symbol at all.

**THE CHECK, and it is robust to this file moving under it** -- recording a tip changes the tip, so
do not chase a tree hash. The branch name is authoritative:

    git diff wip-pre-repair-reorder <new HEAD> -- magic scenes views helpers assets \
        CHANGELOG.md application.fam nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h

**That must be EMPTY.** The reorder changes the ORDER of the shipped work and nothing else, which is
what keeps the hardware bench standing. If it is not empty, something was lost:
`git reset --hard wip-pre-repair-reorder`.

Written HERE and COMMITTED before the rebase, because a previous session recorded branch tips in an
uncommitted edit and lost them to a `reset --hard` made for an unrelated reason.

## ⚠️ TWO OLDER SAFETY BRANCHES — `wip-pre-unaddressed-fold` is the later one

`wip-pre-unaddressed-fold` holds the round as it stood before the stale-comment fold of
2026-09-26; the SHAs below are from AFTER it. The older one:

## ⚠️ SAFETY BRANCH FOR THE EARLIER FOLD — `wip-round15-prefold` = `58ff3e4333467bb9fe192bb3c48914d5d6e4f9ad`

Created 2026-09-26 before folding the review findings into the commits that introduced them. If the
fold goes wrong, `git reset --hard 58ff3e4333467bb9fe192bb3c48914d5d6e4f9ad` restores the round exactly as it was benched.

The tip is written HERE and COMMITTED before the rebuild, because a previous session recorded seven
branch tips in an uncommitted edit and lost them to a `reset --hard` made for an unrelated reason.
Delete this section only after the fold is verified and the branch is deliberately dropped.

## IN FLIGHT: round 15 — BUILT AND BENCHED, nothing pushed, nothing replayed

**25 shipped commits on `iso15693-dev`**, TWELVE fork sync points. 180 host tests, the writing
gate clean, both firmwares warning-free, clang-format clean.

⚠️ **DEV HISTORY WAS REORDERED 2026-09-26**, so every SHA from `bbf3b77` onward is new; safety branch
`wip-pre-repair-reorder`. The repair moved ahead of the survey so it could be sync point 06 on its
own -- it is the round's most consequential fix and it had no visible existence inside a commit about
what a clone leaves behind. Verified content-preserving: `git diff` between the old tip and the new
is EMPTY across every shipped path AND across `tools/` and `.notes/`, so the hardware bench stands.

⚠️ **THE LAST ANCHOR IS THE ROUND TIP** -- 12 since the hardening landed as 11 and 12; before that it
was 08, carrying the addressing AND the arm-model correction AND the self-review's text fixes.
**If any shipped commit is added after the last anchor, it needs a sync point** -- the
sync points once reached only as far as the addressing commit while six shipped commits sat above
them, and nothing but `replay-to-fork.sh`'s own final diff would have caught it.

**THE KNOWN-COUNT GUARD IS BENCHED AND PASSES, both directions** --
[known-count-guard-bench.md](pr-round-15/known-count-guard-bench.md) has the predictions, committed
before the runs, and the results. It fired on `gen-2-card` (reports 28, holds 64) and correctly
stayed silent on `SL2S5302`, which reports 40 and holds 40. The branch the guard ADDS is not
reachable with any card here and was not claimed: every tag advertises the MEMORY flag, and the
timeout path cannot be told from a quiet pass by hand. Harness and mutants cover it.

**THE FAP ON THE FLIPPER IS BUILT FROM `73e9d76`**, installed 2026-09-26 09:23 from
`Momentum-Firmware` on `t5577-deep-read`, API 87.47. So it CARRIES the gen1 addressing (`b312653`).
Recorded because a stale belief about what the device is running has cost a bench session here
before, and because the `experiment-eof-frame` build was on it earlier the same day.

| dev | sync | |
|---|---|---|
| `6ae0978` | 01 | data-block writes carry the card's address |
| `13d1b37` | 02 | the OPTION flag, and the acknowledgement it costs |
| `d835048` | 03 | the gen1 loss claim is made only where there was a loss |
| `ca5c125` | 04 | the clone's identity writes are addressed, and take the OPTION flag |
| `d7edeb1` | 05 | a clone that lands in a gen1 card's UID repairs it |
| `3dc3e32` |  | a clone reports what it left on the card |
| `6695e7a` |  | name the halves that differ, and do not read a register as capacity |
| `33af4db` |  | the notes page says what the user can act on |
| `96fc6fb` |  | why the clone reacts to the registers instead of predicting them |
| `50b99ab` |  | the size note says what the card reports before what it is |
| `bbe08e0` | 06 | the size note names the file where the two counts agree |
| `fd12e75` |  | the gen1 backdoor sequence carries the card's address |
| `2925686` |  | the self-review's first pass -- five stale claims the addressing left behind |
| `361c5d2` |  | the wipe hazard is every gen1 card, not one someone armed |
| `54dceb8` |  | pass 2 -- two numbers that moved, and four paragraphs that were two |
| `88d3099` |  | the wipe's open question points at the evidence instead of repeating it |
| `3765571` | 07 | a boundary comment that named two of three chips, and a release note that grew |
| `319ebf6` |  | the gen2 frames cannot be addressed, and the 2.3 release notes |
| `d04445b` |  | say the gen3 brick was not tried, not that testing missed it |
| `6ac1dcd` |  | the gen3 note drops a tag the reader cannot act on |
| `8ae795c` | 08 | the release notes claim no more than Validation measured, or a user can act on |
| `d2c197e` | 09 | comments from earlier rounds that said something false |
| `b0e8dd2` | 10 | comments from earlier rounds that told their history or repeated a home |
| `ea40479` | 11 | block indices fit what holds them -- the failure bitmap is bounds-checked |
| `26da484` | 12 | -Wswitch sees the write-fail reason switches, and 0 is no longer a reason |

(SHAs after the J1 fold, 2026-09-29 -- see the `wip-pre-j1-fold` section at the top. "TI enforces the
address too" still touches notes only. 08's range holds four commits; 09, 10, 11 and 12 one each.)

⚠️ **THE ROUND WAS REBUILT 2026-09-26** to fold a review pass into the commits that introduced each
fault, so every SHA above is new and the safety branch holds the pre-fold history. Verified: the
rebuilt tree is byte-identical to the old tip with the fixes applied on top, and only the six commits
that needed a fix changed patch-id -- two of those only by context shift. What the fold carried:

- the **five chips** claim, which counted `gen-2-card` as silicon, out of the release notes, a poller
  comment and fork message 01
- the survey comment claiming THREE facts and listing two -- the third comes from a sibling function
- the `CloneComplete` reason described as two triggers at three sites when it has three, the omitted
  one being the only one that fires alone
- a doc block left describing `write_block_retried` while sitting above `write_landed`
- **one behavioural fix**: the size finding needs a claim to be larger than, and `GET SYSTEM INFO`
  does not always make one. Four mutants, all killed; two survived a first pass, one of them because
  the fake tag advertised memory unconditionally and now can be told not to.

Measurements in [pr-round-15/](pr-round-15/): `addressed-writes-measured.md`,
`addressed-writes-implemented.md`, `residue-and-geometry.md`, `controls-2026-09-24.md`.
**Everything measured before the fold passed on hardware** across seven cards covering FOUR identified chips -- TI
Tag-it (x2, by its behaviour), NXP SLIX (x2), NXP SLIX-S, ST LRi2K -- plus `gen-2-card`, whose silicon is unknown.

⚠️ **`gen-2-card` IS NOT "EM-Marin" AND IS NOT A FIFTH CHIP.** That type line belongs to the expired
access credential cloned onto it before the project's first instrumented read;
`tools/tag-inventory.json` records it under `carries_cloned_credential` and says in as many words
that nothing there describes the card. Its own measured facts are 64 physical blocks and gen2 magic.
The label was in `controls-2026-09-24.md` and in the reply draft and is corrected in both -- it is
the same error this round corrected in the release notes, reading a UID-decoded type line as
silicon. Do not reinstate it.

## WHAT TONIGHT SETTLED, so it is not re-opened

- **The standalone EOF works** and closes Gap 2 at the source. Firmware branch
  `iso15693-poller-tx-eof` (API 87.2), unpushed, one chip. The app CANNOT use it: a FAP resolves API
  imports at LOAD time, so naming a symbol the firmware lacks fails the whole load. There is no
  fallback "keyed on API version" — see [firmware-gaps.md](firmware-gaps.md).
- **The empty-frame test is spent** — SOF+EOF is refused, 88 of 88. That is what led to the real fix.
- **`slix2-gold-30mm` has 128 cells mirrored across the 8-bit space**, proved from the write side. Its
  UID sits at 0x10/0x11 and 0x14 is one bit from the V3 config signature. **DO NOT WIPE IT.**
- **The lock model is unsupported AND unfalsifiable with these cards.** Five cards take a bare UID
  write with no unlock; zero acceptances of unlock or commit have ever been observed. But an
  acceptance would have PROVED it and a refusal proves nothing, since a card committed before it
  arrived behaves exactly like one that was never locked. The frames stay in the app.
- **The V1 coin was never a special specimen** — measured identical to `slix-1k-coin18`.

## ⚠️ THE GOLD TAG HAS GEN3'S UID REGISTER, AND IS PROBABLY A VARIANT — `slix2-gold-30mm`

**SETTLED 2026-09-26 by a write.** Block 0x10 is a UID register: `AA BB CC DD` into it moved the UID
to `E0 48 03 00 DD CC BB AA`, the value the mapping implies, and the original bytes restored it.

**It is NOT called a gen3 card anywhere, deliberately.** proxmark identifies an un-finalized V3 by a
signature in 0x14/0x15; this tag is one bit off at 0x14 and seven off at 0x15, further still from
the finalized values, so `hf 15 cfinalize` would refuse it. mfcarroll's purchase explains it: the
seller marked some tags `pm3` and said the unmarked ones need a custom application, and this is one
of the unmarked. A variant, most likely.

**Its UID write needed no custom anything** -- stock `hf 15 wrbl --ua -b 16`. So whatever wants
custom software, it is not the UID register.

⚠️ **STILL DO NOT WIPE IT**, and the reason is now stronger: its configuration state cannot be
placed at all, it is the only tag here that behaves this way, and proxmark's own code treats writing
wrong values to 0x14/0x15 as capable of bricking a tag. A wipe zeroes both.

**NOT RUN, and cheap:** the same reversible probe on the other unmarked tags -- `slix-black-38x25`,
`ti-2k-silver-1/2` -- would say whether the unmarked group shares this mechanism. Read block 16
FIRST; unlike the gold tag they have no committed full sweep to restore from. Full write-up and the
frames in [gen3-candidate-slix2-gold.md](gen3-candidate-slix2-gold.md).

## ~~POSSIBLE GEN3 ON THE SHELF~~ — how it looked before the write, kept for the reasoning

`slix2-gold-30mm` — **DO NOT WIPE IT**. Full write-up in
[gen3-candidate-slix2-gold.md](gen3-candidate-slix2-gold.md).

Blocks 0x10/0x11 READ as its UID in the reversed gen3 layout, and 0x14 reads one bit away from the
V3 config-mode constant. **All of that is reads, and gen3 is defined by a write** -- putting a value
into 0x10/0x11 and having the UID move, which has not been tested. Memory holding a UID copy is
ordinary. The probe's `gen3_signature: false` came from an exact `==` against two constants with no
recorded provenance, so it does not settle it either way. One write to 0x10 would: the UID moves or
it does not, it is reversible from the dump, and finalize at 0x14/0x15 is never touched.

The hazard is the wipe: the release notes carry an attributed report that zeroing 0x14/0x15 bricks an
un-finalized V3 card, those are blocks 20 and 21, and on this card nothing ever refuses a write so the
sweep would not stop early.

**DECIDED: the size note's wording stays.** The survey can be defeated by an aliasing card, which
this one is, but gen3 is out of scope for this PR and the wording is accurate for every gen1 and gen2
card the feature handles. The limit is recorded in the write-up and reported to him in the reply,
placed right after the survey's design rationale. Do not reopen it as a wording fix unless the scope
decision changes.

**It does not change a benched result**, and it does not yet change the squash message's "No gen3
card exists on either side of this PR" either -- that needs the write test. What it does change now
is three unscoped shipped comments claiming a past-capacity block refuses reads, which this card
contradicts whatever it turns out to be.

## The EOF experiment is ANSWERED — SOF+EOF is refused, a bare EOF is untried

Branch **`experiment-eof-frame` = `74bfe45`**, off the round-15 tip. It is NOT what is on the device
— see the head of this file for what is. Know which build is installed before reading a log.

It answers the empty-frame question in [firmware-gaps.md](firmware-gaps.md): can `nfc_poller_trx`
with an empty buffer put SOF + EOF on the air and satisfy the standalone EOF an OPTION write waits
for? Purely additive — it logs `EOF-TEST blk N: trx=… rx=… bytes` and then falls through to the
read-back, so behaviour is identical either way. 165 host tests pass on the branch.

**RESULT: refused.** All 88 probes over 72 blocks returned `NfcErrorTimeout`. The read-back stays.
Write-up in [firmware-gaps.md](firmware-gaps.md), including why the log's `rx=2 bytes` is a stale
buffer rather than a response.

**The device is back on the round build.** The branch is kept as the base for the app half of the
firmware change below, not because anything still needs running on it.

**What the run also established:** a bare EOF is about six lines. The poller encoder writes SOF and
EOF as literal bytes of the 1-of-4 stream — `0x21` and `0x04` — and `poller_tx_common` transmits what
it is handed with parity off, so a standalone EOF is a one-byte frame through the path the poller
already uses. No transparent mode, no bit-banging. mfcarroll is interested in trying it on a branch
of his Momentum fork.

## Can an armed card be LOCKED AGAIN? Nobody has tried, and it is cheap to find out

Raised by mfcarroll 2026-09-26: it is odd if arming is one-way. **The belief that it is rests on less
than it appears to.**

- The register meanings are INFERRED, not known. The defines say so: `written as 0; inferred: unlock`
  and `written as 0x6996; inferred: arms the UID change`. `0x6996` is the value proxmark sends; what
  the silicon does with it is a guess.
- The evidence for irreversibility is that an armed card REFUSES writes to 62/63 — measured in band,
  error `0x10`, **on the LRi2K and on the LRi2K alone**. On the other two gen1 chips the client could
  not separate an error frame from silence.
- **No one has ever attempted a de-arm.** The "do NOT try to de-arm" comment in the wipe is about not
  reordering that sweep's writes; it is not the result of an experiment.

**The experiment is cheap and should come BEFORE the V1 coin.** Three armed gen1 cards are on the
bench — `lri2k-keychain`, `slix-1k-50x28`, `SL2S5302` — and a refused write changes nothing, so
probing costs only time. Worth asking: does the refusal hold on the other two chips; does any value
other than `0x6996` reach 63; does the addressed form fare differently from the unaddressed one.

**Why the order matters:** if anything re-locks a card, arming stops being one-way and the V1 coin
stops being one-shot — which is the single constraint shaping that entire test. Spending the coin
before asking this would be spending it under a restriction that might not exist.

## ~~OPEN QUESTION — should the gen1 backdoor sequence be addressed too?~~ DONE

**SHIPPED as `b312653`, fork sync point 07**, with the fake-tag control fix `757fa5a` beside it.
Write-up: [pr-round-15/gen1-backdoor-addressed.md](pr-round-15/gen1-backdoor-addressed.md). Four
mutants killed, 168 host tests, the FAP builds warning-free against API 87.47. **It has since passed on
hardware** -- see the bench list below, item 1.

The section below is the argument that decided it, kept because the fork message and the reply both
rest on it.

## The argument, as it stood

**mfcarroll argues yes, on the same safety grounds that settled AFI/DSFID, and the evidence is on his
side.** Raised 2026-09-26 after he challenged a comment claiming addressing those frames was
unmeasured. It is not.

**Already measured to work:** the wipe's sweep zeroes 56/57/62/63 through the ADDRESSED path,
re-addressing when the UID moves under it — that is the armed-LRi2K run, 58/58 cleared. And the
clone's conversion path writes 56/57 addressed and watches the UID follow; that is how it detects a
gen1 card on the gen2 path at all. So "it is the magic sequence, leave it alone" does not survive
contact with what this round already does.

**Genuinely unmeasured, and narrower than it looked:**
- unlock (62) and commit (63) ACCEPTED addressed. No run has shown one taken, because an armed card
  refuses them either way. **The V1 coin can answer this** if it has never been committed — add an
  addressed variant to that bench.
- the sequence is fire-and-forget (`send_backdoor_uid_gen1` ignores per-frame results), so addressing
  it needs an inventory between frames. The wipe already does exactly that, so the machinery exists.

**IN THIS ROUND — mfcarroll's call, 2026-09-26, and he is right.** I argued it out on the grounds
that the round was built and benched. That is not a reason: this is the round that introduced
addressed writes to this app *at all*, and it has already changed out of recognition since it
started. Shipping "the writes are addressed, for #251" while the most dangerous frame set in the
feature stays open to a bystander is incoherent, and it is the first thing a reviewer should ask.

The machinery is already here: `iso15693_poller_predict_uid` computes exactly what the UID becomes
after a write to 56, and `iso15693_poller_readdress` takes the new address with that prediction as
its check. So the sequence becomes: address unlock, commit and 56 to the current UID, re-address
after 56 lands, then 57.

Needs bench time on gen1 silicon before it ships — three armed cards are available.

## ~~KNOWN REMAINING WORK — 67 lines of intra-push churn~~ DONE: 2 lines, after the review-2 fold

Both helpers are defined at 02, 03 writes the caveat's final strings, the measurements are final
from 01, and no comment keeps a running list. What remains is in the fork README's residual
section. The section below is the record from before.

Measured on the fork after the release-notes move (110 at the start, 91 after the first pass, 67
now). Two clusters are worth removing and one is not:

- **28 lines, 02 -> 04.** Sync point 02 writes `response_wants_option` plus an inline flags ternary
  and an inline option-noting block inside `write_block_addressed`; 04 extracts
  `iso15693_poller_write_flags` and `iso15693_poller_note_option_wanted` so the identity pass can
  share them. **Fix: define both helpers at 02**, and 04 only adds its own caller. Attempted
  2026-09-26 and ABORTED -- the rebase surfaced the committed conflict markers, which mattered more,
  and starting another rebase right after one had just caused a defect was the wrong trade.
  `wip-pre-helper-move` is the tip from before that attempt.
- **13 lines, 03 -> 06.** The gen1 caveat strings written at 03 and reworded by 06's wording pass.
  **Fix: put 06's final strings in 03.**
- **~17 lines, legitimate.** 01's `readdress` gaining its prediction check at 05, and
  `send_backdoor_uid_gen1`'s signature changing at 07. Both are a later commit genuinely needing
  something the earlier one could not have had, and the README records them.

**The invariant for any attempt** is the one all three rewrites used: the shipped tree must be
byte-identical afterwards. And check every sync point for conflict markers before trusting a
scripted resolution -- `replay-to-fork.sh` now refuses on them, which is how this is caught cheaply.

## WHAT IS LEFT — mfcarroll's read, then the push, then the posts

### THE WHOLE-PR REVIEW, FOLDED -- 2026-09-29

The review is [pr-round-15/app-review.md](pr-round-15/app-review.md). mfcarroll's calls: all three
buckets this round -- round 15's own lines folded into the sync points that wrote them, earlier-round
lines that said something false as a new **09**, and earlier-round history and repetition as a new
**10**; CloneComplete fixed in 06 (success tone and Finish, like over-capacity); `write_identity`'s two
dead buffers removed in 04; the writing gate extended.

- **Safety branch `wip-pre-app-review-fold`** holds the pre-fold history.
- **How.** Round-15 lines went in by `filter-branch --index-filter` in two passes, each rule applied
  from its owning commit to HEAD and required to match exactly once in every one of them; each pass
  verified commit by commit -- identical authors, dates and messages, and only the mapped blobs
  changed. A third pass (amend + cherry-pick) restored one sentence at 08, below. The release-note
  fixes are a new commit in 08's range (`68659cb`, now 08's anchor), since only 08 touches the notes.
- **Where the fold departs from the review, on the record:** "The short-circuit predates this
  feature" is earlier-round text, but 07 rewraps the line it sits on (dropping "ARMED"), so it is
  fixed there: in 10 it changed that line a second time in one push. mfcarroll's call, 2026-09-29. Two round-15 defects the review missed are
  fixed where they began -- `poller.h`'s "gen1's frames carry no UID" (false since 07 addressed the
  sequence) and the write scene's "Two ISO15693 successes ... Both" (06 added a third). E1's sample was
  wrong: the four-run capacity probe was the gen2 card alone, and the plain tag's chip is not
  established as SLI. B3 is scoped to what round 15's bench shows: no card tested has accepted 62 or 63.
- **The maintainer's own words decide four sites.** Every received comment, rounds 2-14, was read
  for keep / checks-out language. **B8 is reverted**: round 14 called "It cannot report the absence
  of a move" "the correct version", and the release note's copy of it stays byte-identical. **B9
  keeps the maintainer's round-14 word** "acknowledged" and adds the OPTION card that acknowledges
  none. **F1 and F2 are left as they are**: round 6 said "worth keeping the rest of this comment as
  it stands" of the file-select routing, and round 5 checked the Back-swallowing engine list claim
  by claim and asked for gen4 in it (its "four protocols" and five engines agree -- USCUID-UL has
  two). Nothing else the maintainer endorsed moved.
- **Verified:** all 23 shipped commits -- no markers, host tests pass, all 71 units compile under the
  firmware's -Werror flags. Host tests at the tip: 171 -- one pinning CloneComplete's tone and Finish,
  which fails on the pre-fold scene, and one pinning that over-capacity with a survey note lands on
  "Clone finished" (every gen1 clone of a larger file does: gen1 keeps reporting its own count, so the
  geometry note fires), which fails if over-capacity is checked first. The writing gate reads the
  tip at 0 findings.
- **The reply** gains a paragraph on 09 and 10 for mfcarroll to check, two claims scoped as the code
  now is (the capacity rule; "could not be addressed on any card tested"), and the notes' growth
  recounted: 117 lines to 157, both counted as the section's body.

**As of 2026-09-29, after the J1 fold: Round 15 is BUILT in the fork: head `846a82eb`, twelve commits,
signed 12 of 12 (all `G`), a fast-forward from `1d411dec`**, the previous build `feed76cf` kept as
`backup-pre-j1-replay`. Trees and messages are identical, commit for commit, to the test replay into
a throwaway clone, and its range-diff against `feed76cf` shows only J1, the message edits and context
shifts. Before it, `feed76cf` was built the same way from `2ccc1afe` (`backup-pre-subject-fix-replay`).
04's and 08's subjects were shortened to fit 80 on mfcarroll's call. 08 keeps "cannot be addressed":
on all four gen2 cards the backdoor refused every addressed form while the cards took addressed
ordinary writes, so it is the measured result, not an absolute to scope.
Its messages are wrapped at 80 throughout, the width the PR's pushed messages use -- mfcarroll found
03's mixed, and edits had left lines broken short in most of them; only whitespace moved.
Everything reviewed so far is folded: review 2, mfcarroll's follow-up, the gen3 wording and cut, and
the whole-PR review above. mfcarroll benched the review-2 build on `lri2k-keychain` 2026-09-28 (Write
UID, a gen1 clone with its geometry notes, the re-clone that converts) and this build's clone
2026-09-29 (success tone, the new gen1 opt-in text) -- all correct.

- **Tested before building:** the replay was first run into a throwaway `--shared` clone, whose tree
  is identical to the real fork's.
- **Every gate passed:** fork messages, each sync point's added comments, the last in full, markers,
  and anchor coverage.
- **Intra-push churn is 2 lines.**
- **Backups:** the builds before each fold are kept in the fork as `backup-pre-review2-replay`
  (`f61285bc`), `backup-pre-followup-replay` (`fe8dc732`), `backup-pre-gen3-wording-replay`
  (`4168c5e0`), `backup-pre-gen3-cut-replay` (`1e79f1e6`), `backup-pre-app-review-replay`
  (`7de7ce82`), `backup-pre-msg-format-replay` (`a4abc966`), `backup-pre-subject-replay`
  (`06812361`), `backup-pre-subject-fix-replay` (`2ccc1afe`) and `backup-pre-j1-replay` (`feed76cf`).
  Nothing of round 15 is pushed.

0. ~~The whole-PR prose review~~ -- run and folded, above. ~~J1, the hardening (11, 12) and T3~~ --
   done 2026-09-29, see the `wip-pre-j1-fold` section at the top.
1. **mfcarroll reads** what changed since `feed76cf`: J1 at its four sites (the poller's chip list and
   fork message 01's copy of it, the OPTION and Validation bullets), fork messages 11 and 12 (new),
   the wrap-only edits in 01, 04 and 07, and [pr-round-15/reply.md](pr-round-15/reply.md)'s paragraph
   on the closing commits, which now covers 11 and 12. mfcarroll's `[👤]` paragraphs are byte-identical.
2. ~~A short bench check of the result screens~~ -- PASSED 2026-09-29, mfcarroll, sound on, on the
   post-fold build (12 moved every write-fail reason up by one; Unset is 0 now). A wipe of
   `lri2k-keychain`, whose sweep reaches 56/57, reported "UID changed / Wiped 58/58 / UID moved. Now
   reads: / 00000000 00000000" -- the armed-gen1 behaviour the release notes describe -- and a second
   wipe, with the UID already zero, reported Success. A card lifted mid-write gave "Write failed" /
   "Card removed before the write could finish". A clean clone ended on Success, and one with
   configuration notes on "Clone finished". Tones correct throughout. mfcarroll restored
   `lri2k-keychain` afterwards.
3. ~~The replay~~ -- done 2026-09-29: `846a82eb`, twelve commits, 12 of 12 signed, test-replayed first.
4. **The push**, on mfcarroll's go-ahead: `git -C ../all-the-plugins push origin nfc-magic-iso15693`,
   a fast-forward from `1d411dec`.
5. **Then the posts**, each on a go-ahead: the reply on #250, then the #255 comment on #255.
6. ~~The dev remote's force-push~~ -- on mfcarroll's go-ahead 2026-09-29, with a lease on the old remote
   head `6575718` (an ancestor of the pre-fold tip, so nothing on the remote was dropped). What lands
   after it is ordinary commits on top, so a plain push carries it.

The older plan below is kept for its record; the order above supersedes it.


**Everything below this heading is DONE unless it says otherwise.** The order from here:

1. **A FINAL REVIEW IN FRESH CONTEXT**, driven by [REVIEW-PROMPT.md](REVIEW-PROMPT.md), which now
   carries fifteen defect classes -- every one of them something that shipped into a draft in this
   round. Classes 12-15 are new on 2026-09-26 and came out of the gen2 bench.
2. **mfcarroll's read of [pr-round-15/reply.md](pr-round-15/reply.md)**, which has caught more than
   any other check here.
3. **The replay**, `tools/replay-to-fork.sh .notes/pr-round-15/fork-messages`, with 1Password
   unlocked so the eight come out signed. ⚠️ **THE FORK IS STALE** -- it holds eight commits built
   from anchors that have since moved three times, so it must be rebuilt before anything is pushed.
4. **Then the push**, which needs an explicit go-ahead every time. Then posting, same rule.

### AFTER THIS ROUND -- the path forward, as of 2026-09-29

1. **Now:** push round 15, then the reply on #250, then the comment on #255.
2. **The maintainer's review of round 15** -- a round 16 only if it asks for changes, or if he takes
   up **the release-notes question the reply asks** (added 2026-09-29, mfcarroll's idea): a short
   2.3 entry in the style of his 2.0, keeping what a user must know before a wipe (a wipe can brick
   gen3, on 0x6r1an0y's report; it can move a gen1 card's UID; one tag in the field at a time), with
   the rest in an `ISO15693.md` beside the changelog, kept current as fixes and gen3 land. **Not a
   straight transfer**: laid out as a reference, not as release notes ("Added" is no heading for a
   standing reference), with room for general gen1/gen2/gen3 notes. His call first; if yes, before
   merge, so nothing released is rewritten. Repo precedent: `SUPPORTED_CHIPS.md`
   (fake_chip_detector), `docs/DESIGN.md` (pocketlab), `.catalog/README.md` beside a changelog in
   three apps. The new file would need adding to `sync-to-fork.sh`, the replay's `SRC_PATHS` and the
   writing gate.
3. **At merge:** post [squash-message.md](squash-message.md) as its own comment, not with a round.
4. **After merge, each its own PR or its own call:**
   - **The host-test harness** -- promised in the replies; the maintainer's `-Wswitch` commit broke it
     once, which is the argument for releasing it.
   - ~~**A small hardening pass, from round 10's self-review**~~ -- MOVED INTO THIS PR 2026-09-29 as
     11 (Y1 + X1) and 12 (Y2), with T3's tests; **Y3 left, by mfcarroll's call**. As it stood: **Y1** `mark_failed`/`unmark_failed` have
     no bound (a `furi_check` makes it structural); **Y2** the reason enum starts at `NotMagic` = 0,
     the scene manager's default state, and `write_fail.c` keeps two silent `default:` cases (the
     maintainer's `e32e6242` covered the poller's write-state switch, not this); **Y3**
     `Iso15693PollerResult` has no mode field; **X1** two implicit `uint16_t -> uint8_t` narrowings,
     the `iso15693_3_get_block_data(source, block)` calls in the clone's data pass and its survey;
     **T3** no test sends a Fail event through the write scene's reason ladder, and the gen1 consent
     screen has none. **P1-P3 shipped in round 13; P4 was declined on diff cost** (the maintainer was
     told); **Y4 is moot.** There is no code-simplification round left -- the whole-PR review's
     comment simplification went into this round as 10.
   - **#255**'s gen3 pre-flight probe, now buildable against a real card (gen3-a).
   - **#251** (the reads and inventories), **#252/#253** (the other protocols' Back trap), and the
     firmware's standalone EOF ([firmware-gaps.md](firmware-gaps.md)), which would let the app collect
     OPTION acknowledgements instead of reading back.
5. ~~**Housekeeping**~~ -- done 2026-09-29: the long comment blocks read against the maintainer's
   comments and none cut; J1 folded; the latch passage kept, because it is not a duplicate (see the
   `wip-pre-j1-fold` section).

**STILL OPEN, none of it blocking:**

- **`lri2k-keychain` block 8** is the one card whose old probe residue was not cleared; `slix-1k-50mm`
  was cleared 2026-09-26 and the rest were already blank.
- **The #255 comment** is drafted: [pr-round-15/issue255-comment.md](pr-round-15/issue255-comment.md), both errors, the
  armed scoping and the latch. Posted with the round, on a go-ahead.
- **`slix2-gold-30mm` and the unmarked tags** -- the reversible UID-register probe on
  `slix-black-38x25` and `ti-2k-silver-1/2` is still unrun, and cheap.
- **gen3-a is benched, 2026-09-29** ([pr-round-15/gen3-a-bench.md](pr-round-15/gen3-a-bench.md),
  inventory + baseline committed): the first confirmed gen3 card, un-finalized config mode. It
  confirmed the gen3 bullet's mechanism -- gen2 backdoor ignored, gen1 opt-in writes 56/57/62/63 as
  ordinary data, no UID move, config signature untouched. The brick stays untested by choice. The
  shipped wording was validated, not changed. `gen3-b..e` stay in the packet and out of scope.

## The round-15 measurement records cite pre-rebuild SHAs

`addressed-writes-implemented.md`, `gen1-backdoor-addressed.md`, `known-count-guard-bench.md` and
`gen1-addressed-bench.md` name commits by SHA from before the five rebuilds. Those SHAs are still
reachable from the safety branches, so `git show` works, but they are not on this branch and
`check-stale-shas.py` is deliberately quiet about them. **Read those files by SUBJECT, not by SHA.**

## The order the bench work was actually taken, kept for what it settled


1. ~~**BENCH `b312653`**~~ **DONE 2026-09-26 and it PASSES** --
   [pr-round-15/gen1-addressed-bench.md](pr-round-15/gen1-addressed-bench.md) has the predictions,
   committed before the first frame, and the transcripts. Plain Success and the WHOLE UID on all
   three gen1 chips, plus the frames alone on `SL2S5302` -- the card that could have killed it, since
   its 40-block claim had kept every addressed frame away from block 56 on SLIX-S silicon.

   ⚠️ **THE THREE GEN1 CARDS ARE LEFT SHARING `E0 11 22 33 44 55 66 77`** unless the restore has since
   been done. Indistinguishable to a 1-slot inventory while that holds. The values to go back to are
   in the bench sheet; identify each card by its tape, not by what it answers.

2. ~~**THE SELF-REVIEW**~~ **BOTH PASSES DONE, 2026-09-26.** Pass 1: `9128500`, `5884a53`, `874c45f`, `c132e29`.
   What it found, all fixed: the reply still saying the backdoor sequences stay unaddressed; "five
   cards take a bare write with no unlock in front of it" (true of two of them); the measurement
   record still concluding that addressing the backdoor had no evidence behind it; a closed
   measurement carried here as outstanding; fork message 04 describing an OPEN QUESTION comment that
   07 deletes; four stale counts; the reply comparing against a version he has never had; and the
   arm-model scope correction below, which turned out to be six sites and shipped as `5884a53`.

   **PASS 2 IS DONE TOO, 2026-09-26.** `5f9ea90`, `0bd9987`, `76b0377`, `d2b85ef`. What it found:

   - **"measured on two chips" was stale as of that morning** -- the SLIX-S run made it three, at
     five sites.
   - **"16 of the 30 lines" is 19**, because the collapsed range grew from five commits to seven.
     The README now records how to re-derive it rather than inviting the next person to quote it.
   - **THE RELEASE NOTES CONTRADICTED THEMSELVES**, 44 lines apart and both inside 2.3: "block
     contents are not compared" against this round's own OPTION read-back, which compares exactly
     that. Found by reading the section end to end; no checker was ever going to.
   - **a blank line between two list items** made the whole Behaviour list loose in CommonMark.
   - **three mangled wraps** -- 115 and 106 characters against a 104 file, 110 against an 80 file.
     Then the fix for one of them produced a 121, caught by re-running the measurement instead of
     trusting the edit.
   - **consolidation**: the reply's gen1 section was four paragraphs doing the work of two, with the
     one-byte-wrong control stated twice and a welded seam; the bench list is a list of RUNS and the
     gen1 item had grown to six lines by re-arguing its own section; two adjacent paragraphs both
     scoping #251 are one; 06's last paragraph did four things in thirteen lines; and the five-card
     unlock evidence had two homes in the poller, 1,300 lines apart.

   Re-derived and correct as they stand: 58/58 on a 56-block LRi2K, seven cards over four identified
   chips, every block count in the bench list, and the fork subjects (88 chars is in line with the
   pushed history's own 87, so that is the project's norm rather than a defect).

   **THE GATE WAS WIDENED INSTEAD OF RECORDING ITS GAPS**, on mfcarroll's call: `replay-to-fork.sh`
   now gates all 95 shipped `.c`/`.h` from the same path list `sync-to-fork.sh` overlays from (it
   read 81 and reported clean for the other fourteen, `nfc_magic_app_i.h` among them, which held one
   of the arm sites); the history rule no longer fires on "is used to"; and ATTRIBUTION is checked
   in both the reply payload and the fork messages, each behind a selftest.

   **AND A TO-POST ITEM:** #255 needs the arm-model correction. **mfcarroll filed #255** -- it is
   ours to comment on, and the reply says the correction will go there.

   **PASS 3 RAN 2026-09-26 on the final state**, at higher effort, after the reorder and the
   loose-end bench had moved things. What it found, and what it adds to the checklist:

   - **AN OPEN ITEM NOBODY OPENED.** The reply told mishamyte #255 was "still yours to call as its
     own PR". He has never been asked, and has never commented on that issue. **This is worse than a
     stale claim**: a stale one was true once, so re-deriving it against the tree catches it. This
     one was never true and survives every mechanical check. Only reading the posted history finds
     it. **Check outward claims about what HE owes against what was actually posted.**
   - **A FORK MESSAGE MUST MATCH ITS OWN TREE.** Sweeping "two chips" to "three" across every
     artifact put a claim in message 01 that its own anchor contradicts. A sweep is exactly the edit
     that forgets a message is prose about a TREE, not about the round.
   - **A NUMBER TRUE OF MOST CASES.** "A target differing in seven of its eight bytes" is six for one
     of the three cards. State the PROPERTY the test relies on -- both halves differ -- which cannot
     drift with the next card.
   - **NARRATION FIXED AT ONE SITE AND LEFT AT ITS TWINS.** Cut from the reply, left in the CHANGELOG
     and fork message 08. The twin-site defect, committed while reviewing for twin-site defects.
   - **SYNC-POINT COVERAGE.** Shipped commits landing above the last anchor reach nobody. Count
     shipped commits against sync points; do not trust the table.

   Also added as gates rather than notes: process narration and the attribution verb "told", both in
   `check-drafts.py`, and `[👤]` paragraphs are exempt from both -- they are mfcarroll's own words
   and he is the first-hand source for them.

   **WHAT MOVED, so the next pass does not chase the old shapes:** every SHA from `bbf3b77` onward is
   new after the repair reorder; there are EIGHT sync points; address enforcement is five of five
   chips, not four; the UID-moves-immediately result is three chips, not two; and "16 of the 30
   lines" is 19.

   The original checklist, which still holds: Round 15 has grown far past what it
   was, and every defect found tonight was found by a human read rather than a checker. Run the
   gates, then the eight read-through questions in [WRITING-RULES.md](WRITING-RULES.md), then the
   checklist that actually catches things here:
   - **a claim corrected in one place and left in its twin** — three separate instances this round,
     all in `iso15693_poller.c`, all from the same commit failing to update a comment twenty lines
     away
   - **an inference about a person written as a report of what they said** — twice tonight, now its
     own rule in WRITING-RULES
   - **a hedge hardened into a claim** — "possibly still LOCKED" became "he called it locked".
     **AND ONE IS STANDING, FOUND WHILE WRITING `b312653` AND DELIBERATELY LEFT FOR THIS PASS.** The
     shipped text describes the wipe hazard as belonging to "a card left ARMED by an earlier gen1 UID
     write". Five cards take a bare write to block 56 with no unlock in front of it, one of them never
     written by this app, so no prior arming is needed and the qualifier **understates the hazard**:
     it is every gen1 card whose advertised count lets the sweep reach 56/57. The reach rule is what
     bounds it, not a card's history. `b312653` corrected the definition site
     (`ISO15693_MAGIC_BLK_UNLOCK`) and the one direct twin it created; the rest is a deliberate
     separate decision, because it touches user-facing release notes and #255's wording:
     `iso15693_poller.c` at the wipe's OPEN QUESTION and at `VerifyWipe`, `iso15693_poller.h:290`,
     `nfc_magic_scene_iso15693_write_fail.c:396`, `nfc_magic_app_i.h:138`, `CHANGELOG.md` 71/74/150.
     **Decide it deliberately, and if it changes, the reply needs a line.**
   - **scope**: name the chip, never the family; say how many CARDS and how many CHIPS
   - **every SHA and number re-derived against the tree at the moment of publishing**, not against
     the round it was drafted for
   - **comment-only proven**, not assumed (`tools/comment-only.py`)
   - **mutation-test from a COMMITTED baseline**, `make clean` in the loop

3. ~~**The reply**~~ -- **REWRITTEN 2026-09-26, still NOT posted and still unread by mfcarroll.**
   [pr-round-15/reply.md](pr-round-15/reply.md) now covers all nine commits (it under-described the
   round by five), leads with the retraction, and states the bench as seven cards over four
   identified chips. Both gates clean. It still needs mfcarroll's read before it goes anywhere.
   `[👤]` marks mfcarroll's own paragraphs and is POSTED as-is -- checked against round 10's
   comment 5652071967, which carries one.

4. ~~**The fork messages**~~ -- **DONE 2026-09-26**, and the counts in this item are that morning's -- the
   tables above are current. Seven sync points for thirteen shipped commits;
   [pr-round-15/fork-messages/](pr-round-15/fork-messages/) has all six and the README's table.
   Gate clean. **06 deliberately collapses seven dev commits** -- the survey, the repair,
   the register-as-capacity fix, the notes-page wording, the comment explaining the repair, and the
   two size-note corrections -- because published separately he would see a geometry note leading
   with a number that matched and then see it corrected twice; 16 of the 30 lines the survey adds to
   the details scene are rewritten later. Splitting the repair out was tried and is
   blocked: it does not apply without the survey. The README records the reasoning and the one
   residual staleness (the Partial release-notes line, stale at 03 and 04, fixed at 05, and not
   closable by reordering).

5. **Then the replay**, `tools/replay-to-fork.sh .notes/pr-round-15/fork-messages`. It resets to
   `origin/<branch>` first, runs the writing gate, refuses on a finding, and does not push. Unlock
   1Password first so the commits come out signed.

**`origin/iso15693-dev` is BEHIND and needs a force-push with lease** -- it holds this session's
pre-rebuild history. Verified to delete no files. That is the dev repo; nothing has gone near the PR.

## ⚠️ Seven safety branches were deleted, and are NOT recoverable

`backup-pre-maintainer-merge`, `backup-pre-reorder`, `backup-pre-round13-reorder`,
`backup-r13-normalised`, `backup-r13-reorder`, `wip-round10-full`, `wip-round15`. A `gc` has since
pruned them, so the tips are gone and no branch can be restored.

**Their content was verified spent BEFORE deletion and that verification still stands:** five were
patch-equivalent to dev by `git cherry`, `wip-round15` was tree-identical, and `wip-round10-full` was
the superseded FIRST build of the gen1 B-round, whose findings were recovered into
[gen1-hardware-findings.md](gen1-hardware-findings.md) at the time. So nothing of value is missing --
only the ability to go back and look.

The tips WERE recorded here, in a write that was never committed before a `git reset --hard` for an
unrelated rebuild discarded it. That is the trap this file already warns about two ways. **Commit a
notes change before any reset, including one you are about to make for a different reason.**

## The measurements behind it

**All five CARDS accept addressed WRITE BLOCK** -- four identified chips plus `gen-2-card`, whose
silicon was never captured. Not "five chips": counting that card as silicon is the error corrected in
the release notes, a poller comment and fork message 01 this round. Measured 2026-09-24, full
transcripts and frames in
[pr-round-15/addressed-writes-measured.md](pr-round-15/addressed-writes-measured.md). That file is the
input to the implementation; read it before writing code.

The design is settled by measurement rather than by argument:

- always-addressed is safe -- `gen-2-card` was the one that could have blocked it and does not
- the WIPE must re-inventory and re-address after a write to 56 or 57 lands, because the UID moves
  immediately and the stale address gets silence. Confirmed on NXP SLIX and ST LRi2K, the latter
  being the chip the armed-gen1 hazard was reproduced on
- ~~the gen1 backdoor sequence stays UNADDRESSED~~ -- **reversed 2026-09-26, see `b312653`**
- the SDK cannot do it: `iso15693_3_poller_write_block` hardcodes the flags and
  `iso15693_3_write_block_response_parse` is internal, so we need our own builder and response check

**Nothing is outstanding on that measurement, and the last gap closed 2026-09-26.** `SL2S5302`'s
mis-addressed control was run 2026-09-24 -- silence, bracketed by a reader either side. TI Tag-it was
the one card left without one, because the reason first given for skipping it ("it refuses
unaddressed writes, so it already discriminates") was withdrawn the same day: what it refuses is a
write without the OPTION flag. It was sent a mis-addressed frame with OPTION set in both, so the
address was the only variable -- silence to the wrong UID, `00 78 F0` to the right one. **Five of
five enforce the address.**

**Two controls also passed** before this, in
[pr-round-15/controls-2026-09-24.md](pr-round-15/controls-2026-09-24.md): EM-Marin still wipes 64/64,
and gen1 clears 28/28 ordinary data blocks unaddressed. Both were recorded as unmeasured prerequisites
in the round-10 finding.

**#251 is three defects and this closes one.** The 1-slot inventory and the missing STAY QUIET are
untouched, and the issue's worst consequence -- the post-wipe UID check answered by a bystander --
cannot be fixed by addressing at all. Do not report this as closing #251.

## WHAT IS ACTUALLY BLOCKING NOW

**Addressed writes, and the gen1 caveat gate with them.** The reply no longer offers him a scoping
choice: shipping ISO15693 support that cannot write a TI Tag-it is not a later problem, so this
belongs in this PR. **What makes that card writable turned out to be the OPTION flag, not the
addressing** -- see the correction at the head of the finding. Both are recorded in
[pr-round-10/unaddressed-write-finding.md](pr-round-10/unaddressed-write-finding.md).

Raised with him and awaiting his call: the protocol menus keeping their cursor across a fresh scan.
App-wide, one dispatch point in `magic_info.c`, six lines. We offered to implement it.

## ⚠️ TWO RULE FILES, READ THE RELEVANT ONE FIRST

[WRITING-RULES.md](WRITING-RULES.md) before anything goes outward.
[BENCH-RULES.md](BENCH-RULES.md) before anything is measured -- extracted 2026-09-24 after shipping a
five-card addressed-write measurement in which one card never received its control, so for that chip
"it enforces the address" and "it ignores the flag entirely" were indistinguishable.

### WRITING-RULES.md

Built this round because the rules existed and we broke them anyway -- they were 264 lines of prose
in this file, ordered by when we learned them. They are now ordered by WHICH ARTIFACT you are
writing, with a gate (`tools/check-writing.py`, wired into `replay-to-fork.sh`, which refuses to
replay on a finding) and, more importantly, **eight read-through questions the gate cannot replace**.

Every defect this round was caught by a human read, not a checker: comparisons against work he cannot
see, a section he could not act on, a conclusion that did not follow, a question that was no longer
open, restating a lesson he taught us, a whole reply section duplicating its own thread, and -- after
fixing that -- narration about where we had moved it. A fourth stale heading got through a checker
written to catch stale headings.

## Where things stood before

**mishamyte pushed `a27d187d..749f10e6` to the PR branch himself** — five prose fixes answering
round 11, a six-commit sweep of the whole PR, and an rx-buffer fix. **Four are behavioural**, which
is new; he had only pushed prose before. He invites rebasing, rewording or dropping any of them.

**ALL TWELVE ARE ADOPTED into dev** (`7bd719d`), since dev is what sync-to-fork overlays from and the
fork is no longer downstream of it. Our round-13 gen3 text, twin-site fixes and "copied exactly" came
off with them as moot — he reached the same conclusions independently.

**⚠️ THREE BEHAVIOURAL COMMITS ARE UNBENCHED.** `0c5a7d69` (gen1_attempted set before the frames go
out), `4d20f06e` (CFG geometry clamp), `62b60e2b` (menu cursor). He has no ISO15693 hardware and says
so. mfcarroll is at the bench 2026-09-21. The fourth, `749f10e6`, needed no bench and was verified
against the firmware source here: bit_buffer_copy's furi_check, the 64-byte poller buffer, and all
five in-tree callers passing instance->rx_buffer as both source and destination.

**Drafts in [pr-round-14/](pr-round-14/)** — reply and seven thread replies, checker-clean, every
cited SHA verified against the branch. NOT posted, NOT reviewed by mfcarroll.

## What we added on top of his twelve

| dev | |
|---|---|
| `26df4dd` | P1 the clock cut |
| `a58de0b` | P3 the clamp -- his 4d20f06e made it three callers, not two |
| `155675a` | P2 the succeeded-blocks figure |
| `be1795e` | the sweep's attempted-count invariant |
| `ada1d98` | the layout note's x convention, which answers the P4 wrapper question |
| `9b68f3e` | the reach rule's third gate -- offered as a fix TO his 5c97e16f |

Two of the old ten folded away: the naming rationale now lives in P1's helper doc, and 07 became a
correction to his commit rather than a parallel rewrite.

## ⚠️ HIS -Wswitch COMMIT BROKE THE HOST HARNESS

`e32e6242` added `furi_crash`, which was not in `tools/hosttest/fakes/furi.h`, so the poller stopped
compiling there. Fixed in the adoption commit. **Nothing on his side could have shown this** — the
harness is dev-only and never syncs. It is now the concrete argument for releasing it, and the reply
says we will put it up as its own PR rather than adding it here.

## Still unsigned

1Password was locked through this work. Fork commits are signed at replay time; re-run
`replay-to-fork.sh` with it unlocked before any push and confirm the count.

## Where things stood before

**HIS ROUND 12 ARRIVED 2026-09-17** and is fully addressed: seven findings, all verified against the
tree before fixing, all fixed. Round 13 combines those with the simplification pass, since his review
landed before P1-P3 went out. Draft reply, thread replies and fork messages in
[pr-round-13/](pr-round-13/); his review is cached in `pr-round-13/received/`.

**NOTHING HAS BEEN PUSHED OR POSTED, AND NONE OF IT HAS HAD MFCARROLL'S READ.** That read has caught
more than any other check on this project, so it is the gate. Seven fork sync points, no reorder
needed, and **zero intra-round churn** -- the first round with none.

**⚠️ THE ROUND-13 DEV COMMITS ARE UNSIGNED.** 1Password was locked while he was away. Fork commits
are signed at replay time, so re-running `replay-to-fork.sh` with it unlocked gives a signed chain --
check for `7 of 7`. Do NOT re-sign the dev commits: it rewrites history and renames every
`NN-<sha>.msg`.

## What round 12 found, and the pattern under it

All seven were correct and all were verified here rather than taken on trust. Two of them are the
shape worth remembering:

- **The trim escalated a claim and deleted its evidence in the same edit.** The gen3 bullet came to
  say the gen1 opt-in writes "reach the same blocks" as the registers a wipe bricks -- false, since
  gen3 keeps its UID at 0x10/0x11 and its signature at 0x14/0x15 -- while the same commit removed the
  only record of those addresses. Unfalsifiable and wrong arrived together.
- **Three of the seven were one defect: a claim corrected in one place and left in its twin.** The
  consent screen fixed and the release notes not; the in-band error code scoped at its definition and
  not at its two users. Worse than uniform wrongness, because the reader has to work out which half
  is current. **When a claim appears in N places, fix all N or none.**

The reach rule had been wrong in four consecutive rounds, each time in a new corner, so it is now a
closed form stated once in the poller (`L = max(A + 7, claim - 1)`; 56 reached iff `A >= 49` or
`claim >= 57`) with the CHANGELOG carrying only the qualitative half. His catch that a landing READ
resets the run -- not just a landing write -- is one this session would have missed.

## Where things stood before that

**Pushed `536e0d4f..a27d187d`, a fast-forward of 7.** PR head confirmed via the API, comment-only
outside the CHANGELOG, `fap_version` still 2.3, nine files, all seven signed.

**Posted:** the main reply as [issuecomment-5703486508](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5703486508),
and all seven thread replies, each verified by `in_reply_to` AND by the root's file matching what
the reply discusses.

**THE SIMPLIFICATION PASS IS BUILT AND UNPUSHED** — P1-P3, four shipped-file commits' worth of work
in three, plus two dev-only. Draft reply and fork messages in [pr-round-12/](pr-round-12/). **Not
pushed, not posted, and not reviewed by mfcarroll** — drafted while he was away, and his read is the
check this project has relied on most. Nothing goes out until he has had it.

**The reply says this should NOT merge yet**, and why: TI Tag-it HF-I Plus refuses unaddressed
WRITE BLOCK with error 0x01, and every ISO15693 write this app sends is unaddressed. Not a
regression — the write path is unchanged since the passing August run and the EM-Marin control still
passes. It reframes #251 from a bystander hazard into a compatibility limit.

## THE PLAN HE HAS BEEN GIVEN, in the order stated in the reply

1. ~~**The comment cut, WITH the release-notes trim**~~ — **DONE, pushed as round 11.** 84 comment
   lines out, code `+0 −0`, and the 2.3 section 177 → 115. Numbers in "Where things stand" below.
2. ~~**The simplification pass**~~ — **BUILT, UNPUSHED.** P1-P3 from
   [pr-round-10/self-review/FINDINGS.md](pr-round-10/self-review/FINDINGS.md); P4 stays DECLINED and
   he has been told so. Three shipped commits, each with a test, each mutation-checked. Draft reply
   and fork messages in [pr-round-12/](pr-round-12/). See "The simplification pass" below.
3. **Addressed writes**, and the gen1 caveat gate with them — both recorded in
   [pr-round-10/unaddressed-write-finding.md](pr-round-10/unaddressed-write-finding.md).
4. **Re-test on device**, including the TI cards.
5. ~~**The squash message**~~ — **REWRITTEN TO THE FINAL STATE 2026-09-26**, 96 lines at 80 columns.
   The old draft was badly stale and the round-15 reply now OFFERS it, so it had to be true: it still
   said a gen1 card "latches a written UID only on the next power-up" (no latch on any of three
   chips), that gen1 was validated on a single card (five, over three chips), that the backdoor
   registers "accept writes WITHOUT acknowledging" (a parse, not a measurement -- they answer 0x10
   or 0x0F), and that writes are unaddressed (they are addressed). None of this round's feature work
   was in it. Post it as its own comment at merge, not with the round.

mfcarroll leans toward keeping the addressing work inside this PR and said so in the reply; the
scoping decision is with mishamyte.


## Where things stand

**Everything through round 11 is PUSHED and POSTED** — fork head `a27d187d`. **The PR is waiting on
him, not on us.**

Round 11 was his review of the round-10 push: seven findings, nothing blocking. Two needed no fix
(the cut had already removed the cost footnote and the "claim of 49" sentence). The rest became the
seven fork commits:

| | |
|---|---|
| `9b80e348` | the comments state the constraints and stop arguing for them |
| `ab647375` | the write-fail card-lost term is defensive, and says so |
| `7350fbd3` | three mangled comment wraps, and a note naming one mode of three |
| `0bd82c19` | the no-latch result covers three chips, not gen1 silicon |
| `8d419a09` | the gen1 consent screen says why it has no validation line |
| `44a2a0c1` | the sweep's reach depends on reads, not on the card's claim |
| `a27d187d` | block 57 needs one more silent block than 56, and the clock is a third exit |

**The cut, measured on the tree he has:** 84 comment lines out, code `+0 −0` (preprocessed output
byte-identical for every shipped `.c`/`.h`). Surface 1,544 → 1,460, block mass 740 → 610 across
40 → 36 blocks, 2.3 section 177 → 115.

**The estimate published in round 10 was 250–370 and the answer was 84.** Fourth time a comment-pass
estimate has missed in the same direction. Do not quote a projected line count to him; quote the
measurement after the fact, taken against the tree he can open.

### The simplification pass — BUILT, UNPUSHED

Item 2, and the first code change since the comment cut, so none of it has the "code bytes unchanged"
safety net. Each landed with a test and each was mutation-checked from the committed baseline.

| dev | |
|---|---|
| `a5070a1` | **P1** — the clock cut decided in one place, with both halves of the reason |
| `5ea5a5b` | **P2** — the three succeeded-blocks subtractions say why they saturate |
| `998e4b4` | **P3** — one clamp for block size, against the macro rather than a buffer |
| `d6036be` | dev-only: the fake tag modelled a latch the hardware does not have |
| `6ac73fa` | dev-only: the fake-flash analogy leaves the dev tooling too |

**All three were the same shape**: one rule spelled in two places, each copy carrying part of the
reason and neither carrying all of it. That is worth knowing before P-items are picked for a later
pass — it is a better predictor of value than line count.

**Sync points need no reordering this time**: dev order already matches fork order, three shipped
commits, one decision each. [pr-round-12/fork-messages/README.md](pr-round-12/fork-messages/README.md)
has the mapping and, more importantly, what those messages must NOT claim — every test is in
`tools/hosttest/`, which does not sync, so a message citing them would describe a change absent from
its own diff. That is the round-11 defect, avoided by putting the test evidence in the reply instead.

**DECIDED, and do not re-open without him: P1-P3 is NOT published as a side branch.** It was
considered while he was away. A loose branch cannot be merged, needs a comment to explain it (which
lands in the reviewer's queue exactly as a push would), and splits his attention while he is mid-review
on round 11 — the same objection as pushing, moved sideways. Nothing degrades by waiting.

**Mutation-testing gotcha, cost real confusion:** a scripted revert can land in the same second as the
object file, and make then treats the target as up to date indefinitely — a stale binary reporting the
MUTANT's result against restored source. `make clean` belongs in the mutation loop. The harness's own
dependency tracking is fine; this is a make property, not a Makefile defect.

### What round 11 cost us, and why

Three things went wrong and all three are the same shape — a claim that was true when written and
went stale when the tree moved under it.

- **The cut deleted the carve-out he quoted as correct** (`a card that refuses every write but still
  serves a read never accumulates a run`). Restored before it shipped, so he never saw it.
- **The cut dropped a negation**: `A cut wipe records nothing above its cut` became `records above
  its cut`, which inverts the claim. The sense pass missed it; the review workflow caught it.
- **Three fork messages claimed more than the final tree supported**, including one asserting an
  armed 56-block LRi2K "satisfies neither" conjunct in a commit whose point is that the rule IS a
  conjunction. Caught by re-checking every claim against the tree before the replay.

**The rule that comes out of it:** re-verify every number and every claim against the tree at the
moment of publishing, not against the round the sentence was drafted for.

### Churn control — how the seven were chosen

`replay-to-fork.sh` syncs the TREE at a named dev commit, so sync points decide what he watches
happen. Fifteen dev commits touched shipped files; seven became fork commits. **Dev history was
REORDERED** so the restorations sit immediately after the cut — otherwise 01 would have shipped the
dropped negation and a later commit would have put it back. Reordering was verified by tree hash:
identical before and after.

Residual churn is 11 lines, all one paragraph written in 01 and rewritten later for a DIFFERENT
reason (the latch paragraph, the carve-out, one CHANGELOG line). The harmful kind — delete-then-
restore — is absent. See [pr-round-11/fork-messages/README.md](pr-round-11/fork-messages/README.md).

### Prose rules he made us learn, round 11 edition

**No internal process in anything posted.** Not when something was cut, not that an earlier commit
in the same push already removed it, not that we broke and restored something he never saw broken.
He reads the delta between the round he reviewed and the next one. "Cut" is a complete answer.

This applies to COMMIT MESSAGES too: `replay-to-fork.sh` takes its message from the `.msg` file, and
round 10's were reflowed copies of the dev messages — which is how he came to quote one. Round 11's
dev messages were NOT copyable and the fork messages were written fresh.

### THE BENCH RUN IS DONE — 4/4 gen1, and it corrected one of round 9's own fixes

All four pm3-marked candidates are **gen1**: `slix-1k-50x28`, `slix-1k-coin18`, `slix-1k-50mm` (NXP
SLIX, 28 blocks) and `SL2S5302` (NXP SLIX-S, 40). With the LRi2K that is five cards across three
chips, and the CHANGELOG says so now.

**Two corrections came out of it, and both were standing facts here.**

1. **The backdoor addresses are NOT memory.** "Writable memory above the advertised count" was the
   wrong reading of the LRi2K, by analogy with a gen2 card's over-claim. With `--capacity-max 70`
   searching past them on all four new cards, every one stops at its advertised count and
   56/57/62/63 answer **no read, ever**. Write-only registers outside the memory map. A 28-block
   gen1 card has 28 blocks.
2. **The wipe reaches the UID registers from a claim of 49, not 57.** The sweep keeps writing past
   the claim until `ABSENT_RUN` blocks answer nothing, so a card silent from A is attempted through
   A+7, and a landing write resets the run — so 56 and 57 both go at 49. Measured 44→52, pinned by
   tests at 48/49. **The 57 figure was his, from round 9 thread 01, adopted without checking.** The
   band it wrongly called safe is 49–56.

   It runs toward safety: only the 56-block LRi2K reaches its own UID registers under a wipe. The
   other four trip out seven blocks past their claim.

**And a harness bug worth not repeating.** `pm3_exec` decoded pm3 stdout as strict UTF-8. `hf 15
rdbl` prints an ASCII rendering of the block, so block 0 of any **NDEF-formatted** tag — which opens
`E1`, the CC magic number — made the decode throw, and the caught exception was recorded as a FAILED
READ. Three bench sessions went looking for an RF cause for `slix-1k-50mm`. The tell was in the
first capture: every failure named the same byte offset (472/474), and RF does not fail
deterministically to the byte. Fixed with `errors="replace"`; the hex column the parser uses is
ASCII and survives. Same class as the `gcc -fpreprocessed` and `grep -q` false passes.

### The cut, as its own round — DONE 2026-09-15, see "Where things stand". Kept for its reasoning.

**The consent screen no longer reports gen1's validation status at all** — that was folded into the
first round-10 commit rather than shipped as a round-trip, since correcting it and then removing it
inside one batch is the intra-batch churn he has flagged twice. What the user consents to is the
write and its blast radius; the sample size is a fact about the project and lives in the release
notes.

**The bench run is now about the CHANGELOG claim, not the screen.** "validated on hardware, on one
ST LRi2K card" is a claim about evidence, and that is where more cards change the text.

Four `pm3`-marked tags have **never had a write probe run**, and the seller said those are the
proxmark-writable ones:

| tag | blocks | state | gen1 probe |
|---|---|---|---|
| `SL2S5302` | 40 adv / 40 phys | blank, restorable | safe — 56/57/62/63 above measured capacity |
| `slix-1k-50mm` | 28 adv | `[0,1]` unread | safe |
| `slix-1k-50x28` | 28 / 28 | blank | safe |
| `slix-1k-coin18` | 28 / 28 | blank | safe |

**The probe is a real test, not a foregone negative.** gen1 needs 56/57/62/63 to exist, and on all
four those sit above the *measured* capacity — but a read-based capacity figure is only a LOWER
BOUND, and `lri2k-keychain` is exactly the case that proves it: 56 advertised, and it took writes at
56/57/62/63 anyway. So the probe discovers whether these answer the gen1 backdoor at all. (RESULT:
all four do, and none of them has memory there — see the corrected finding below.)

The four unmarked tags (`slix-black-38x25` 28 blocks, `slix2-gold-30mm` 79/82, `ti-2k-silver-1/2`
64 each) are expected to refuse — the seller said they need a custom application. Note the last
three have the backdoor blocks genuinely IN range, so the probe is destructive there; all are blank
or restorable, but run them last and restore from baseline.

**Then the cut**, scoped in [pr-round-10/comment-cut-measurement.md](pr-round-10/comment-cut-measurement.md).
Its projection was ~250-370 lines out of `iso15693_poller.c` alone; **the shipped figure was −84
across the surface**. The advice that held: do not go after the header, whose 64% is per-field
contract and the least compressible comment here. The advice that did not: the opportunity in the
poller was a fraction of what was projected.

**ROUND 10 = the self-review mishamyte asked for**, run 2026-09-12 with
`/pr-review-toolkit:review-pr` (seven agents) over the reconstructed FINAL-STATE PR diff — 27
files, +3922 −45, built by replaying `sync-to-fork.sh` onto the PR merge-base so it covers the
unpushed commits rather than only what he can see. Everything is in
[pr-round-10/self-review/](pr-round-10/self-review/): `SCOPE.md`, `FINDINGS.md`, the diff, and the
session's own mechanical results.

### The finding to lead the reply with — it explains rounds 8, 9 AND 10

**`tools/gen1-staleness.py` could not see a single one of the eight stale gen1 sites that survived
the B-round**, including two consent-screen strings that told users gen1 was untested. Two
vocabulary gaps, both the same mistake: the patterns match the phrasing that was *already fixed*.

- `latch(es)?\b` misses `latched` — the word is at three shipped sites, and the scanner flagged the
  two that are CORRECT.
- the validation pattern matches "validated", never "tested". `2edb202` removed exactly three sites,
  all spelled "NOT hardware-**validated**"; the survivors all spelled it "not hardware-**tested**".

A pattern list written by reading the sites you just fixed encodes their vocabulary and is
systematically blind to the ones you missed — and then reports the job done. Fixed, plus a SELFTEST
list of known-stale phrasings asserted to match (runs on every invocation, refuses to scan if it
fails), plus a loud exit when given no files, since a bare run printed nothing and exited 0.

### The one behavioural fix, and it is a real user-facing gap

**A wipe that loses the card never said the identity check had not run.** It returns before
`VerifyWipe` is entered, `has_details` sent CardLost to its default arm, and "UID not re-checked"
is reachable only through Details. By then the sweep has written 56/57 — which on an armed gen1
card ARE the UID — so the card could be gone and its identity with it, and the user was told only
that they had removed it. `has_details` now covers CardLost when `wipe_mode && !uid_verified`; the
note's wording is chosen by route (a card-lost wipe never reached a field reset); and the block
list is SUPPRESSED there, because the sweep's own comment says a lifted card "can surface as a pile
of blocks that wouldn't clear". Four tests, each mutation-checked to kill exactly one.

Second code change: the wipe's live progress denominator moved BELOW the geometry guard, so a card
reporting `block_size == 0` can no longer carry its own unverified claim into a terminal event.

### What the review did NOT find, which is worth saying in the reply

A full correctness pass found **no functional defect** — memory, buffers, tick wraparound,
tail-drop arithmetic, state-machine completeness, Back handling and the poller/scene thread
boundary all verified, the "widget copies its string" constraint checked in the SDK rather than
assumed. Nothing treats a refused backdoor write as proof it did not land. All 13 reason codes
reach a titled screen. Every numeric figure outside two off-by-ones matched.

### Still OPEN from round 10 — deliberately not done

These were in the review but NOT in the five items that were actioned. **None of them has ever been
put to mishamyte** -- checked across the round 10-14 replies 2026-09-26 -- so "decide before the
reply" has now been deferred five times. They are ours to propose or drop, not his to wait for, and
round 15 deliberately does not raise them: a reply that says the work is ready should not also open
six new items. Decide them as their own pass:

- **Y1** `mark_failed`/`unmark_failed` write the bitmap with no bound; the invariant is held by four
  call sites. A `furi_check` would make it structural.
- **Y2** the reason enum has three silent `default:` absorbers and `NotMagic == 0`, which is also
  the scene manager's default state. `…Unset = 0` plus dropping two defaults gets `-Wswitch`.
- **Y3** `Iso15693PollerResult` has no `mode` field though six of sixteen fields are mode-scoped.
- ~~**Y4**~~ **MOOT since 2026-09-26.** It named the hazard as "a gen2 ref into
  `build_gen1_frame`", and `build_gen1_frame` no longer exists -- the gen1 addressing routed those
  frames through `iso15693_poller_build_write_frame` like every other write. The two define families
  are still both `uint8_t` and still both spelled `BLK`, so a milder version could be restated, but
  the specific defect is gone.
- **X1** two implicit `uint16_t -> uint8_t` narrowings, the only two in the file that lack a cast.
- **P1-P3** three simplifications worth doing (the duplicated pass-cut is the real one: two write
  points for `pass_truncated`/`pass_cut_block`, each holding half the rationale).
- **P4** the twelve `widget_add_string_multiline_element` calls — a wrapper works and the y
  rationale is already hoisted to the file header, so the old objection is weaker than the notes
  said. **Declined on diff cost at round 10**, not on principle. Tell him, so it is not re-derived.
- **T3** zero coverage on the reason-code selection ladder and on the gen1 consent screen — the
  highest-consequence routing in the feature and the only screen where a wrong button destroys a
  card. Both are ~60 lines of test against harnesses that already exist.

### Verified on the current tree, 2026-09-12

| check | result |
|---|---|
| host tests, from `make clean` | **114 run, 0 failed** (108 -> 114) |
| Momentum `dev` @ slix, forced rebuild | **zero nfc_magic warnings** |
| Unleashed `unl092-base` | **zero nfc_magic warnings** |
| clang-format, shipped | **95 files, 0 need formatting** |
| intra-batch churn, all 13 round-10 commits | **none** |
| comment-only | 3 comment-only, 7 code/CHANGELOG, 2 dev-only tools |

⚠️ **`git checkout -- <file>` ate an uncommitted fix during mutation testing this session**, exactly
as the rule below says it would. And GNU Make 3.81's whole-second timestamps reported a wrong test
red and then a dirty baseline. **Mutation-test only from a committed baseline, and always
`make clean`.**

## Round 8 and earlier — where things stood

PR #250, `nfc_magic_dev` on branch `iso15693-dev`.

**THE GEN1 B-ROUND IS BUILT — eight shipped commits on dev, on top of `56e8f49`, NOTHING PUSHED.**
Seven are comment-only (proven with `tools/comment-only.py`) and one is the CHANGELOG. Zero C code
changed. Both firmwares warning-free, 108 host tests, clang-format clean, all signed.

**IT WAS BUILT TWICE.** The first build assumed the latch was unmeasurable; a second bench session
measured it, and four of those seven commits stated the superseded model. The round was reset and
rebuilt rather than patched, because a series that asserts a thing and retracts it two commits later
is the intra-batch churn he has flagged twice. Nothing was pushed, so it cost nothing.

⚠️ **AND THE RESET SILENTLY TOOK THE BENCH FINDINGS WITH IT.** `git reset --hard` to before the round
also dropped the notes commit holding Findings 5-7 — the only record of the session. Recovered from
the safety branch, which is the whole reason to make one before a reset. **Never reset a round without
`git branch wip-<name>` first, and check what notes commits sit inside the range.**

**THE TWO REVERSALS, both in [gen1-hardware-findings.md](gen1-hardware-findings.md) as Findings 5-7:**

1. **There is no power-up latch.** A written UID takes effect IMMEDIATELY — an INVENTORY in the same
   field session already returns it. The app was built on the opposite premise and four comments plus
   two CHANGELOG entries said so. **The `NfcCommandReset` STAYS** — it is free, it re-activates for a
   clean read, and gen2's UID is in a register space this was never tested against — but it is
   belt-and-braces now, not load-bearing, and the comments say which.
2. **"The backdoor registers accept writes WITHOUT acknowledging" was a PARSE, not a measurement.**
   62/63 answer with error 0x10; `hf 15 wrbl --ua` renders that as `( fail )`, which session 1 read as
   an absent ACK. Do not reinstate it. What replaced it is stronger and does not need silence: on an
   armed card BOTH register writes are refused and the UID moves anyway, so a poller acting on those
   returns would abort a run that worked.

**THE SURFACE ESTIMATE WAS WRONG, and in the wrong direction.** The findings doc predicted "20-40
lines out before C starts". It came out **positive**. A measurement is LONGER than the hedge it
replaces — it carries the card, the date and the observation. Do not budget a B-round as a reduction;
C is the reduction.

**#255 still carries the softer "moved UID" wording**, which this round makes wrong rather than merely
soft. The earlier do-not-reopen decision is worth revisiting: "left with no valid identity" is a
different warning from "the UID moved".

**ROUND 8 SHIPPED 2026-09-11.** Fork pushed `dbc6e4fa..09778b6d`, a fast-forward of **16 commits**,
all signed. Main reply at **issuecomment-5641879307**, **all 16 thread replies posted**, and the #251
scope correction at **issue 251 issuecomment-5641885668**. Everything verified byte-identical against
the drafts after posting. PR now reads **108 commits, 27 files, +4,123 −45**.

His round 8 was `COMMENTED` and closed "nothing blocking beyond the regression". **The standing
`CHANGES_REQUESTED` is stale — 2026-08-16 — and his last five reviews are all `COMMENTED`.** Do not
read the badge as an objection.

**13 of the 16 were comment-only**, proven with `tools/comment-only.py`. The three that were not: the
`write_confirm` title regression, `ISO15693_POLLER_MAX_BLOCKS` across three files, and the CHANGELOG.

What the reply commits us to, so a later session does not contradict it:

- **The case for finishing before the merge is made, and made as HIS interest** — the PR squashes, so
  tidying inside it costs `dev` nothing, while the same work afterwards is a second PR and a diff of
  pure comment churn against a released app.
- **The volume arithmetic is conceded in public**: the cut removed 95 comment lines, round 7's own
  corrections put 85 back, this round another 25. Fifteen lines worse off than before the cut.
- **The harness is promised as its own PR**, with the sizing that rules it out of this one.
- **The gen1 round is put back in front of him**, on the grounds that the card now exists.
- **A CHANGELOG correction is offered separately** if he merges before the gen1 round —
  `CHANGELOG.md:145` still tells users the gen1 path was never tested against hardware, which is now
  outright false rather than cautious.

**ROUND 7 SHIPPED 2026-09-11.** Fork pushed `d31f5162..dbc6e4fa`, a fast-forward of **19 commits**, all
signed. Main reply at **issuecomment-5628507918**; **all 20 thread replies posted**. The delta is three
reviewable groups: the cut (8), the corrections (10), the gen3 warning (1). His upstream key-cache work
was verified untouched by all 19 before the push, and `fap_version` 2.3 — his own conflict resolution —
is preserved.

What the reply commits us to, so a later session does not contradict it: the volume objection is
**conceded with numbers**, the C and D passes are **proposed and argued**, the CHANGELOG is **named as in
scope**, and he is **asked not to merge on the current PR body** and to let us propose the squash message.

**#255 is left as it stands — decided 2026-09-11.** Its text still carries the softer "moved UID"
wording the CHANGELOG no longer does, but Brian's comment is on the issue and says the stronger thing,
so a reader gets the real cost. Editing it would mean posting for little gain. Do not reopen.

Round 6 for reference: reply recorded at 2026-08-22T18:08Z, so use that date, not the 08-20 some of
these notes carry.

**Every physical tag is in [tag-inventory.md](tag-inventory.md)** — what it is, what it measured before
anything wrote to it, and whether it has ever been written to. Read it before touching hardware, and
`python3 tools/iso15693_magic_probe.py --identify` to find out which tag is actually on the antenna.
Labels live on paper, UIDs live on silicon. [tag-inventory.md](tag-inventory.md) opens with the
groups that cannot be told apart by looking, computed from each tag's `form_factor` rather than
listed by hand -- there are four, and two of them nobody had noticed.

**The round-7 comment cut was BUILT: sixteen commits on dev, on top of `04d5f8a`, all signed.** (It
shipped long ago — this paragraph is the round-7 state as it stood. NOT the 2026-09-15 cut, which is
a separate, later pass that shipped as round 11; see "Where things stand" at the top. Two different
deltas have been called "the comment cut" and BOTH are now pushed.)
Eight of them are the cut itself; the rest are the corrections and citations found afterwards, plus notes. Results and the full argument are in
[pr-round-7/comment-cut-plan.md](pr-round-7/comment-cut-plan.md) under "EXECUTED". Headline: comment
**−95**, code **+11**, seven of eight commits comment-only and proven so, zero intra-batch churn, both
firmwares warning-free, host tests **101 → 106**.

**mishamyte has NOT replied.** He was asked to pick the cut's scope and whether he wants the gen3
pre-flight probe (#255) as its own PR. The cut was done on the stated preference (option 1) rather than
waiting, which was the plan. If he asks for the narrow version, this delta is wider than he wanted —
that is the known risk and it was taken deliberately.

**ROUND 7 LANDED 2026-09-07 — `COMMENTED`, 20 threads, nothing blocking.** Read
[pr-round-7/ASSESSMENT.md](pr-round-7/ASSESSMENT.md) first: it is against the PRE-CUT code, our cut
closes exactly ONE of his threads (and not in the shape he proposed), and it PRESERVED FOUR claims he
has now flagged — it shortened comments without re-checking them. He also merged upstream
`dev` into the PR branch (`d31f5162`) as housekeeping.

**The WHEN decision is resolved: he has replied, so the cut can go up framed as a partial answer to
Round 7** rather than as an unprompted round. That was the better option and it is now available.

### The finding to lead with, whenever it goes

The ratio is the wrong metric and this pass has the numbers to say so: **−102 comment lines moved the
surface from 37% to 36%**, because removing comment lowers numerator and denominator together. Reaching
`gen2_poller.c`'s 9% by deduplication is arithmetically impossible. The metric that DOES track the
defect is how many places state the same fact — **repeated comment phrases went 135 → 34**. Concede the
real part: ISO15693 carries ~9x the comment per line of code, and `gen2_poller.c` is 875 lines, so the
gap is not a size artefact. Argue about what the residue IS, not that the gap is imaginary.

Fork `nfc-magic-iso15693` = **`049029c9`**, 41 commits, all signed and GitHub-verified, fast-forward with
no force at any point. PR shows 72 commits. Dev `iso15693-dev` = **`9da594f`**, clean, all signed.

**His Round 6 was `COMMENTED`, not `CHANGES_REQUESTED`** — the first time in six rounds. He verified all
three Round 5 blockers by tracing them, tabulated all thirteen reason codes through the button rule, and
answered our open question: keep the capacity guard AND keep 10s, because a clone-specific budget is the
real fix and not this PR's.

Posted: [#issuecomment-5381850091](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5381850091)
plus **22 threaded replies covering all 22 of his threads** — every one verified byte-identical by
re-fetching. Drafts in [pr-round-6/reply.md](pr-round-6/reply.md) and
[pr-round-6/thread-replies.md](pr-round-6/thread-replies.md); his review verbatim in
[pr-round-6/received/](pr-round-6/received/).

What went up — ten commits, one decision each, **zero intra-batch churn**:

1. `d43c0a8` **blocking** — the Fail guard judged on counts alone, so a cut clone that accepted nothing
   reported "no data block took". One conjunct: `!instance->pass_truncated`.
2. `8c9f9b3` the fourth `block_is_empty` site, the dead `+ over_capacity` term, the `cut == advertised`
   off-by-one
3. `79f1629` the bitmap set/clear pair
4. `08c45ac` `{reason -> title}` — the re-scoped render table
5. `8f64fa7` the back-fill is clone-only; two truncation archetypes unfused
6. `3171e66` the budget's cost arithmetic (it inverts), and `COUNT_OF`
7. `8b40231` three result-screen claims, and why `WipeUidChanged` withholds Retry
8. `bc0dc69` three user-facing CHANGELOG errors
9. `0850837` two minor claims and a 155-column comment line
10. `9da594f` cite #255 from the gen1 open question and the gen3 entry

## The comment cut — what it was, now that it is built

This was committed to, in writing, at the end of the posted reply. It is the last item on the deferred
queue and the round's own evidence made it the priority:

**Fixing eleven false comments cost +117 comment against +28 code, and took `iso15693_poller.c` from 43%
to 44%.** Every round that corrects a claim adds the explanation that makes it correct, so the metric moves
the wrong way even when each edit is right. Three of his eleven threads were comments contradicting *other
comments in the same delta*; two were user-facing. **That is a duplication problem, not a density one** —
the same fact in three places drifts in two.

The reply asked him to choose the scope and stated a preference. **Option 1 is what was built**, on the
stated preference, because he had not answered:

1. **The full ownership model.** The event enum owns the outcome contract, the `ISO15693_MAGIC_BLK_*`
   defines own the wire facts, `gen1_optin.c`'s strings own the user-facing gen1 consequence,
   `ISO15693_POLLER_PASS_MAX_MS` owns the budget rationale. Everything else cross-references instead of
   restating. This targets the mechanism that produced Round 6.
2. **Narrow** — only the facts that have already drifted twice: the back-fill's scope, the two truncation
   archetypes, the prefix property, the cost figures. Leaves the mechanism intact.

**Do it as its own delta with nothing else in it**, so the diff reads as one decision. That was promised in
the reply.

Also in that pass, per his notes:
- ~~the **compact-UID formatter's four copies**~~ — **DONE, struck 2026-09-12.** The helper
  `iso15693_info_cat_uid` exists (`iso15693_info.c:283`, declared `iso15693_info.h:27`) and those
  four sites are now call sites of it. No duplication remains; do not re-plan it.
- the twelve `widget_add_string_multiline_element` calls varying only in `(x, y)`. Deliberately left in
  Round 6: those y values carry the line-budget arithmetic he measured for us in Round 4, and they should
  move in the comment cut rather than be buried in a table.

**The plan is written up in [pr-round-7/comment-cut-plan.md](pr-round-7/comment-cut-plan.md)** — the
measured per-file ratios, the seven known duplicated facts as a work list, what must NOT be cut, and how to
verify it. Start there rather than from this section.

**Tests first, then the cut.** `tools/hosttest` now covers the result screens and the write scene's
routing, so a comment-only pass is verifiable as behaviour-preserving rather than read-and-hoped. If the
cut touches code, update the tests in the same commit.

## ⚠️ THE SQUASH MESSAGE IS THE COMMIT MESSAGES — corrected 2026-09-08

**A previous version of this section said the PR body becomes the squash message. That is WRONG.**
Verified by diffing #238 / #236 / #244: the squash body is GitHub's `COMMIT_MESSAGES` default —
`<title> (#N)`, then `* <headline>` + body per commit, joined by GitHub's `---------`. #258's body is
9949 bytes; its squash message is 799. So **commit messages survive a squash, concatenated.**

The pack does squash-merge (every `dev` commit has one parent; #258's four commits are not ancestors).
That part holds.

**The live hazard: the merger overrides the default.** mishamyte hand-wrote #258's 799-byte message
rather than take the 4064-byte concatenation. **Our 20 fork-bound commits concatenate to ~728 lines** —
so an override is likely, and then the messages are lost.

- **ON THE LIST: write the proposed squash commit message and POST IT AS A PR COMMENT when merge nears.**
  That is the whole mechanism, and two earlier readings of it here were wrong. The PR DESCRIPTION is
  never used — #258's body is 9949 chars against a 799-char squash message. The default is the
  concatenated commit messages, and he overrode it on #258 by writing his own in the merge box. So the
  only lever is to hand him one: he is the person clicking merge, and a comment is how he gets it.
  Draft ready in **[squash-message.md](squash-message.md)** — 62 lines, house style verified by
  measurement rather than assumed (#258's longest line is 79 columns, not the 72 an earlier note here
  claimed). Post the payload between the `~~~~` markers, as a comment, when merge nears.
- **The PR body is NOT being rewritten — settled 2026-09-12.** It is what a reader of the PR page
  sees and it never enters git history. Step 2 of the pathway used to say "worth doing on its own
  merits", which contradicted this; that step is struck. The squash message is the only one of the
  two worth writing.
- Consequence for the comment work: BOTH tests are live. "Would a maintainer editing this line, offline,
  need it?" is the strong one. "Would it be at home in the commit message?" is valid again but a WEAK
  home — it survives only if nobody overrides.

**Do not re-inherit the wrong premise:** comment VOLUME is not his objection. Checked against all 86
review events — he has never asked for less comment, only for accurate comment. The 42%-vs-10% figure and
the cut itself are ours. It IS an outstanding public commitment (round 6: "not treating it as optional"),
which means **the scope is ours, not his.**

## THE PATHWAY TO RELEASE, in order — settled 2026-09-08, step 3 added 09-09

1. **Push + post Round 7** (built, verified, awaiting a go-ahead).
2. ~~**Improve the PR body**~~ — **STRUCK 2026-09-12. Decided: we are NOT rewriting the PR body.**
   Only the squash message. The body never enters git history (#258: a 9949-byte body against a
   799-byte squash message), so it is the one artifact where effort does not compound. This step
   contradicted the "optional polish, not a priority" line in the section above, which is the
   reasoning that holds; the contradiction stood for three rounds and was acted on once. Do not
   re-open.
3. **Offer him the squash message** — same file. **Timing: not yet.** The trigger is the PR nearing
   merge — an approval, or him asking whether it is done — because the message has to describe the final
   state and the C/D passes will change it. **DECIDED: it stays in `.notes/` and gets pasted as a PR
   comment.** Never a tracked file: "remove before finalization" is a step that gets forgotten, and this
   way we keep what a comment cannot give — a diff across rounds. Do not re-open this. Condense first:
   ~190 lines now, #258's comparable is ~20, so 60-100 is the target.
4. **The gen1 B-round** — [gen1-hardware-findings.md](gen1-hardware-findings.md). Eight of its nine
   items are comment corrections, so it is a **B pass, and B precedes C/D.** It also SHRINKS the surface
   (inference -> measurement removes the hedging), so it makes the comment passes easier rather than
   harder. The card is a reusable fixture — restore, test, restore — so it is not one-shot.
5. ~~**C — does it need to be there?**~~ **DONE, shipped as round 11**: 84 comment lines out, code
   `+0 −0`, surface 1,544 → 1,460. The 1,576 / 787 / 43 / 250-370 figures this step used to carry
   were measured against a tree that never shipped and the estimate missed by 3x; both are recorded
   in [pr-round-10/comment-cut-measurement.md](pr-round-10/comment-cut-measurement.md).
6. **D — can it be correctly simplified?** SMALLER than C and separate from it. The live items are
   the self-review's P1-P3: the duplicated pass-cut (two write points for the two fields that gate
   Retry, half the rationale at each), the three saturating `total - bad` subtractions whose shared
   constraint is explained 600 lines away, and the block-size clamp spelled two ways. **P4 —
   wrapping the twelve `widget_add_string_multiline_element` calls — is DECLINED on diff cost**, and
   the old objection recorded against it (that the per-site y values carry the line budget) does NOT
   hold, since that rationale is hoisted to the file header. Say so rather than let it be re-derived.
7. **THE RELEASE NOTES — added 2026-09-13, previously untracked. RUNS WITH STEP 5, not after it**
   (decided 2026-09-13). The cut deletes the code-side copies of reasoning the release notes also
   carry, so splitting them leaves the two artifacts briefly disagreeing and gives him two tightening
   deltas to read instead of one. The 2.3 section is **178 lines
   against 14 and 19** for the two entries before it, i.e. half of CHANGELOG.md for one feature.
   Same discipline as C: what a user needs about the card in their hand stays, the reasoning goes.
   This was discussed across several rounds and never written down anywhere, which is why it kept
   being re-raised.

Nothing in 3-6 is a merge blocker, but there IS a reason to want all of it BEFORE the merge, and it
is mechanical rather than a taste for polish: **the PR squashes, so everything inside it collapses to
one commit and the tidying costs `dev` nothing.** The same work afterwards is a second PR, a second
review, and a diff of pure comment churn against a released app — more expensive for him than for us.
State it that way round; "nothing blocks a merge, and I am not asking you to hold" gives away an
argument that is actually in his interest.

**Do not over-claim what he said about gen1.** The line is "on the tags: worth having, but nothing
here waits on them. None of the three blocking items below needs a gen1 card" — that is about the
BLOCKERS being reachable on gen2, said while the tags had not arrived. It is not "merge without the
gen1 round", and earlier notes here paraphrased it that way. The card exists now, which makes that
round bounded work rather than an open wait, so it is worth putting back in front of him rather than
treating his old answer as settled.

The order is still a preference. Running C before 4 would have C protect text the gen1 round is about
to replace.

## Deferred DELIBERATELY, not forgotten — 2026-08-22

**A full `/code-review` pass over the PR was scoped and NOT run**, to save tokens in a fresh weekly
window. Revisit in a burn window, ideally after the next round lands so it reviews the final code once
rather than twice. The scope decided at the time, and the reasoning, so it does not need re-deriving:

- **Run it over the ISO15693 surface, max effort** — `iso15693_poller.c/.h`, `iso15693_info.c/.h`, the
  six ISO15693 scenes, `scene_write.c`, `write_confirm.c`, `file_select.c`, `nfc_magic_app_i.h`. About
  4,000 lines.
- **Not the full `origin/main..HEAD` range** (74 files, 7,146 insertions, 221 commits): most of the rest
  is light touches on gen2/gen4/USCUID-UL that six rounds have already passed, so it roughly doubles the
  spend to re-review code that is not new.

### Both documentary items are now CLOSED — 2026-08-24

Surfaced 2026-08-22 while checking whether any code change was outstanding. Neither was a functional fix
and both are done.

1. **#255's mitigation claim had a hole** — it described the post-wipe UID re-read as unconditional while
   `iso15693_poller.c`'s `wiped == 0` short-circuit skips it entirely, which is the path an armed gen1
   card would need it on. Fixed in three places: the comment at the short-circuit (`65e741a`), the
   CHANGELOG's wipe entry (`7570ef7`), and the issue itself —
   [#255 comment 5389538269](https://github.com/xMasterX/all-the-plugins/issues/255#issuecomment-5389538269),
   posted 2026-08-24 and verified byte-identical to
   [pr-round-7/issue-255-followup.md](pr-round-7/issue-255-followup.md). That comment also discharges
   mishamyte's Round 6 "file it rather than fix it" ask, which the Round 6 reply had answered with a code
   note while saying "Filed" — a loose end he could have found.
2. **#251 is now cited**, at the two choke points where its frames are built rather than at the functions
   the issue names: `write_block_retried` (every block write; the SDK sets no ADDRESSED flag and no UID)
   and `verify_inventory` (every UID read-back; single-slot, so a bystander can answer). `27d939b`. The
   release-notes half was decided in favour of shipping it — `8e9ba69` adds it to "Validation (at 2.1)"
   beside the gen3 entry. Re-verified against the current SDK rather than trusting the Round 2 report:
   `iso15693_3_poller_i.c:237` write_block, `:132` inventory, both still as filed.

**Nothing is owed outward now.** Everything else waits on him.

### Surfaced by the round-14 self-review, UNFILED and not in this round — 2026-09-17

Both are pre-existing, neither is touched by round 14, and neither was on any list before now, which
is the only reason they are written down here. Checked against the notes first: the third item the
review raised -- `NothingWiped` having no `uid_verified` route -- is ALREADY settled above and in
[#255](https://github.com/xMasterX/all-the-plugins/issues/255), so it is not repeated.

1. **The gen2 CFG frame programs an UNCLAMPED block size.** `cfg_blocksize = (uint8_t)(sys->block_size - 1)`
   takes the source's raw value while every data write goes through `iso15693_poller_clamp_block_size`
   and is capped at 32. Reachable only through a hand-edited `.nfc`: the SDK's loader checks
   `block_size > 0` and nothing else, and that is byte-identical in Momentum, Unleashed, RogueMaster,
   Xero and official, so 1..255 can arrive. A source claiming 200 programs 199 into the card's
   geometry register while writing 32 bytes a block, leaving a clone whose reported geometry its own
   contents do not match. Not a safety issue -- the write path is clamped -- but the two should agree
   on one number, and the clamp's new note now makes the asymmetry easy to see.

2. **The unusable-geometry return blames the card for a refusal it was never asked for.**
   `iso15693_poller_wipe_blocks` returns 0 on `advertised == 0 || block_size == 0` before a single
   frame goes out, and that lands on the same `wiped == 0` branch as a card that refused everything --
   where the screen reads "No blocks could be cleared: the card accepted no zero-write." It accepted
   nothing because nothing was sent. Nothing is logged at that return either, so a card reporting no
   geometry is indistinguishable from a card refusing writes, both on screen and in the log. It also
   carries `wipe_advertised` (set above the guard) against `blocks_total` 0 (set below it), so the
   screen can show a claim of 64 beside a total of 0 with nothing explaining the gap. A third branch
   in the `nothing_wiped` body and one `FURI_LOG_E` would close both halves.

## Open, waiting on him

- **The scoping call on addressed writes** — whether they land inside this PR or as a follow-up.
  mfcarroll leans toward inside and has said so twice; the decision is his. This is what "should not
  merge yet" rests on.
- **The comment cut's scope** — asked at the end of the round-10 reply, and round 11 did not answer
  it either way. The cut shipped regardless; the open part is whether he wants more taken.
- ~~**Whether he wants the gen3 pre-flight probe as its own PR.**~~ **NEVER ACTUALLY ASKED, and this
  file carried it as "awaiting his call" for five rounds.** Checked 2026-09-26 across every main
  reply and thread reply from rounds 10-14: the only mention of #255 in any of them is round 10
  correcting its wording. Round 15's reply nearly shipped "still yours to call as its own PR", which
  asserts a decision he was never given. An open item nobody opened is worse than a stale one -- it
  cannot go stale, because it was never true. Filed as
  [#255](https://github.com/xMasterX/all-the-plugins/issues/255) (`type/enhancement`, filed 2026-08-20)
  carrying both register hazards: gen3 is detectable via the `0x14`/`0x15` signature, armed gen1 is not.
  The code cites it from both sites.

## Settled in earlier rounds — do not re-litigate

- **The capacity guard stays, and so does 10s.** He answered this in Round 6: dropping the guard trades a
  fabrication the user cannot check for one they can, and 20s doubles the Back-swallowed window on every
  card to buy a diagnosis on one shape of card. A clone-specific budget is the real fix and not this PR's.
- **The render table is `{reason -> title}` and it is DONE.** The fuller `{reason, title, body}` form is
  dead and he agrees: only four of twelve bodies are static, and `wipe_stopped` arriving dynamic moved the
  ratio further away. Do not revisit it.
- **`WipeUidChanged` withholds Retry deliberately**, and the reasoning lives at the branch ordering in
  `scene_write.c` where a reader will look for it. Not an oversight.
- **`NothingWiped` has no `uid_verified` route, and that is filed rather than fixed** — gen1, no card, and
  the `wiped == 0` short-circuit predates this PR.

- **The clone has no consent screen on the happy path, and that is correct.** He agreed explicitly in
  Round 5: consent deferred to the moment a destructive path becomes real, naming the actual
  consequence, beats a fixed warning describing a hazard gen2 ISO15693 does not have. The wipe/clone
  asymmetry is answered by a wipe's only product being destruction. **The gen1 opt-in carries the real
  consent and does not change.**
- **The host harness is out of this PR**, and whether `base_pack` grows a test directory is xMasterX's
  and mishamyte's call, not ours. He flagged it to them rather than answering for them.
- **Helper names carry the `iso15693_poller_` prefix** even where he proposed a shorter name, to match
  the other statics in the file. He has seen this and not objected.
- **Pass C's confirm-scene fold was hardware-verified 2026-08-11** (Write UID and Wipe confirm screens);
  the plain clone confirm and USCUID-UL wipe text need cards nobody on the PR has, and are safe by
  construction — for a non-ISO15693 protocol the new conditions are false and control falls through the
  pre-existing chain unchanged.

## Rules that cost us real time — cumulative, all rounds

- **THE COMMIT TEST IS NOT THE COMMENT TEST, and reasoning is not "process narration" wherever it
  appears.** The two artifacts answer different questions for different readers. A comment answers
  *what must I not break when I edit this line?* — which is why "measured on device rather than
  counted" fails there: nobody can act on it. A commit answers *why did this change?* — so the test
  is **does it tell a reader of the diff something the diff cannot show?**

  Two categories pass that test while failing the comment test, and they are the same two things a
  diff structurally cannot show:

  - **Removals.** A diff shows what left, never whether it went somewhere else. `02-c0aa63a`'s note
    that "NOT hardware-validated" was dropped *because the caveat already lives on both gen1 entry
    points, and what went was a dangling `see the PR note`* is evidence about the current state of
    the code. Without it a reviewer correctly reads a vanished safety caveat as a quiet deletion.
  - **Rejected alternatives.** `17-26602eb` says the rewrap does not reopen the cut's
    do-not-rewrap decision, and where the boundary falls: churn-avoidance holds at 104 columns, not
    at 189. That forestalls "you said you weren't rewrapping."

  **Do not use "it gets squashed anyway" as the licence** — that has it backwards. Squashing makes
  commit messages LESS durable. GitHub's `COMMIT_MESSAGES` default would concatenate all of ours into
  the squash body, but they run ~728 lines, so whoever merges will more likely write their own summary
  (as he did for #258) and they leave git entirely, surviving only on the PR page. The licence is that
  explaining the change IS the commit's job. It is also why load-bearing facts cannot live only there:
  the queue-hang argument and the 2026-08-04 measurement stay in the code, and the PR description
  rewrite is on the list.

- **VERIFY WHICH TREE A BUILD CLAIM IS ABOUT.** Two ways this went wrong on 2026-09-09, both silent.
  `Momentum-Firmware` is a working tree on branch `t5577-deep-read`, **1287 commits ahead of
  `origin/dev`** with unrelated LF-RFID work — a green build there says nothing about stock. Stock
  Momentum is **`Momentum-Firmware-slix`** (branch `dev`, clean, API 87.1). And
  `unleashed-firmware/applications_user/nfc_magic_dev` had been a real **directory**, not a symlink, so
  it silently went three commits stale and a "both trees clean" claim covered a copy with the gen3
  warning missing from it. **Now a symlink** (`../../nfc_magic_dev`, matching Momentum's), verified by
  rebuilding through it: same 165,952-byte FAP, zero warnings, `tools/` still excluded. `applications_user`
  is gitignored in Unleashed, so the symlink is not a tracked change.

  Stock trees to build against, and what to report:

  | | tree | branch | API |
  |---|---|---|---|
  | Unleashed | `unleashed-firmware` | `unl092-base` @ `3c9be0fd` | **88.4** — the SDK he builds with |
  | Momentum | `Momentum-Firmware-slix` | `dev` @ `8ed809f` | **87.1** |

  And **`clang-format` is not on PATH** — the toolchain's is at
  `Momentum-Firmware*/toolchain/arm64-darwin/bin/clang-format` (18.1.8), used with
  `--style=file:<firmware>/.clang-format`. Running the bare command makes every file report as needing
  format, which is the false-FAIL twin of a `grep -q` false pass and just as uninformative.

- **A REPLY TO A FINDING NEEDS AT MOST THREE THINGS: the disposition, anything HE got wrong, and
  anything WE found while doing it.** Everything else is padding to the person who wrote the finding.
  Round 7's drafts broke this in two opposite directions, nine replies between them, and both come from
  the same instinct — writing to show we engaged rather than to say what he does not already know.

  **Restating his reasoning back at him.** The absent-run reply spent four lines explaining his own
  finding to him. Eight more did it at 9–20% overlap. Detect it by measuring shared 7-grams between his
  comment and the reply, **excluding code spans** — shared identifiers are legitimate, shared prose is
  not. Legitimate exceptions exist and the measure will flag them: quoting text we *added* or *deleted*,
  and confirming reasoning on a thread where he wrote "my error, please revert" — there, showing we
  followed the argument rather than the instruction is the point.

  **Claiming his correction as ours.** Worse, because it reads as taking credit. *"Fixed. Four
  qualifiers, not three"* — he wrote "There are four qualifiers, not three". Detect it by pulling every
  correction-shaped phrase (`not two|three|N`, `rather than the N`, `your figure`, `actually`,
  `miscount`) and checking each against his actual text. Round 7 had one of these, three legitimate
  corrections stated once too often, and one correcting a claim he never made at all — "there are two of
  those preambles, not three", when he had said two.

  When a phantom correction has real substance under it, **reframe, do not delete**: that one became a
  note that three sites share the condition line and only two share the behaviour, which is worth
  knowing and was never his claim to be wrong about.

  **THE RULE COVERS THE MAIN REPLY TOO, AND THE 7-GRAM SCAN WILL NOT FIND MOST OF THE PADDING.**
  Learned again in round 8, where the rule was already written down and twelve of sixteen thread
  replies plus the main reply broke it anyway. The overlap scan flagged exactly ONE (16.7%); the other
  twelve measured **0-5% and were still padded**, because paraphrasing his reasoning in fresh words
  shares no 7-grams. So the scan catches quotation, not restatement. **The only check that works is
  per-paragraph, by hand: does this tell him something he does not already know, and can he act on
  it?** Run it over every paragraph of every reply including the main one.

  Three shapes to delete on sight, none of which the earlier version of this rule named:

  - **Describing the diff he is about to read.** "Two comment lines say why the title is not taken
    from `is_wipe`", "The line now says why the derivation is written out". He reads the diff commit
    by commit; a prose summary of it is strictly worse than the diff.
  - **Justifying our own action to him.** "It is new information about the hazard's size, not a
    restatement of what is already filed." Nobody asked. Say what was done.
  - **Explaining why we are telling him something.** "Laying it out because you are close to ready and
    I would rather you know what is still coming than discover it." State the thing.

  And the tell that started the round-8 sweep, which the user caught: any sentence opening **"Worth
  noting"**, **"It is worth"**, or **"One thing to note"** is almost always about to narrate our own
  process. Grep for them before posting.

  What the trim is worth: thread replies 202 -> 161 payload lines, main reply ~1,200 -> 1,030 words,
  and nothing of substance lost — every cut was his own reasoning, a description of a diff, or
  process narration.

- **NEVER CITE A DEV-REPO SHA IN ANYTHING HE WILL READ.** Caught 2026-09-09 in the round-7 reply, which
  cited `3199bb9`→`7883953` and `58de6ba`→`b312deb` for its churn figure. Both halves were unresolvable
  for him, for two independent reasons: `7883953` and `b312deb` had been rewritten by the fold rebase and
  are unreachable, and even the LIVE dev hashes never appear on the fork — `sync-to-fork.sh` overlays its
  own commits, so the branch he reads has entirely different hashes. **Cite commit SUBJECTS instead**;
  they survive the sync, and he greps for them anyway. This is a whole error class the dev-repo/fork split
  creates and nothing in the workflow catches it — no build, test or format check looks at prose. Sweep
  every draft before posting:

  ```bash
  grep -oE '\b[0-9a-f]{7,9}\b' .notes/pr-round-*/reply.md .notes/pr-round-*/thread-replies.md | sort -u
  ```

  Same trap in reverse: a fork SHA is meaningless in a dev-repo commit message. And any SHA-bearing
  figure goes stale the moment a commit is added — the same bullet claimed "6 lines, both instances" when
  a later commit had made it 9 lines in three. **Re-measure every number in a draft immediately before
  posting**, not when it is written.

- **"WE CANNOT TEST THAT" GOES STALE THE MOMENT HARDWARE ARRIVES, AND NOTHING CHECKS IT.** Caught by
  the user 2026-09-11 in the #251 draft, which repeated the issue's original "it would need two
  ISO15693 tags and would destroy data on the second". That was true on 2026-08-11, when there was one
  tag. **There are fourteen now, every one with a captured baseline and restorable from it, and most
  of them blank** — see [tag-inventory.md](tag-inventory.md). So the two-tag test destroys nothing
  permanent and is available today.

  This is the same class as the gen1 hedging, but `tools/gen1-staleness.py` does not catch it: that
  scans for claims about the gen1 PATH, not for claims about what the bench can do. **Before repeating
  any "not staged / needs a card we do not have / would destroy" line, open the inventory and check
  the count and the restorable column.** The phrases to grep for are `not staged`, `no card`, `needs a
  card`, `would destroy`, `cannot test`, `nobody on the PR has`.

  And when correcting one of these in a comment on our own earlier report, **say the reason expired
  rather than quietly dropping it** — the old text stays visible above the new comment, so an
  unexplained reversal reads as inconsistency.

- **A read-based capacity measurement is a LOWER BOUND, not a capacity, and so is a search that hits its
  own ceiling.** Proven twice on 2026-09-08. `tools/iso15693_magic_probe.py`'s capacity probe reported
  "82 real blocks" on a card advertising 79 when every block up to its search bound answered — 82 was
  `advertised + 2 + 1`, the probe's own limit. And on the gen1 card it measured 56/56 while the app's
  write-based wipe found 58, under-detecting by two. `ISO15693_POLLER_WIPE_MAX_BLOCKS` says exactly why:
  only a WRITE settles whether a block exists. The tool now says "lower bound" in both cases.
- **A no-ACK is not a refusal on the gen1 backdoor registers.** They accept writes without answering —
  observed 2026-09-08, previously an inference from proxmark's source. So no write-based probe of those
  registers can have a meaningful negative, and a gate built on one will confidently mislead: that cost
  two wrong answers on the same card before it was understood. Only the full sequence plus a
  power-cycled UID re-read is conclusive.

- **A comment earns its place only if it records something the code cannot show AND is not already
  stated elsewhere.** Report added/removed comment vs code per commit with `tools/comment-ratio.py`;
  a commit adding more comment than code is going the wrong way. **But do not use the ratio as the
  target** — the 2026-08-22 cut removed 102 comment lines and moved the surface 37% -> 36%, because
  removing comment lowers numerator and denominator together. Count SITES PER FACT instead; the
  duplicate-phrase scan in the round-7 plan is the tool for it (135 -> 34 over that pass). The strong comment reduction is **item 6**, which is held — items
  1-4 came out at comment +4 / code −51, and the +4 is three constraints that had nowhere else to live
  (tick wraparound, don't-reuse-`is_wiping`, the widget copies its string so the early free is safe).
  Naming a value is often what makes the wrong refactor look attractive, so that is exactly where the
  constraint has to be written down.
- **`gcc` ON THIS MACHINE IS CLANG, AND CLANG HAS NO `-fpreprocessed`.** Caught 2026-09-11 when the
  user asked why the reply claimed "every commit touches shipped code" for a round that is mostly
  comments. The comment-only check run inline as
  `gcc -fpreprocessed -dD -E -P <file> | md5` **errors out and emits NOTHING**, so comparing two
  empty outputs reported "identical" for every pair — including a commit that changes a ternary and
  adds a line. A false PASS of exactly the `grep -q` shape, and it was quoted in a commit message as
  "proven so rather than asserted".

  **Use `tools/comment-only.py`**, which finds a real GCC in
  `../Momentum-Firmware*/toolchain/arm64-darwin/bin/arm-none-eabi-gcc`, and **exits non-zero if the
  preprocessor emits nothing** rather than treating emptiness as a match. `--range A..B` classifies
  a whole round. This is the check the C deletion pass promises ("code bytes unchanged"), so it has
  to be the thing that cannot pass by accident.

  The round-8 answer it gives: **13 of 16 comment-only**, the exceptions being the title regression,
  `ISO15693_POLLER_MAX_BLOCKS` across three files, and the CHANGELOG.

  **And the reply bullet it replaced was wrong for a second reason:** "none is notes-only" names a
  category the fork does not have. `.notes/` and `tools/` exist only in the dev repo, so on the PR
  every commit is in the pack by construction and the claim is both invisible and, read as English,
  the opposite of true. **Shipped-vs-dev-only is OUR bookkeeping; never put it in a reply.** What he
  wants there is code-vs-comment, which tells him where to spend attention.

- **A test that has never been observed to FAIL is not evidence, and neither is a build system whose
  dependency tracking has never been observed to fire.** Mutation-test anything new: break the fix the
  test covers, watch the test go red, restore. On 2026-08-18 two `write_identity` tests passed against
  deliberately broken source -- not because the tests were weak, but because `make` re-ran a stale
  binary. `tools/hosttest`'s depfile rule compiled the test and its fake in ONE `cc` invocation sharing a
  single `-MF`, so the fake's dependency list overwrote the test's and no depfile ever named
  `iso15693_poller.c`. It had carried a comment asserting the opposite for weeks. Nothing was masked --
  a from-scratch run of all 83 passed -- but several "green" claims made that day were worth less than
  they looked. Fixed by compiling each TU separately; verify with the two greps in the Makefile.
- **Do not leak process into artifacts that describe the present.** No "this used to be duplicated" in
  a comment, no "no longer" in a CHANGELOG for an unshipped feature. The rule goes in the comment, the
  incident in the commit message.
- **Audit commit hygiene before pushing.** For each pair of commits, does a later one remove a line an
  earlier one in the batch added? Diff each with `--unified=0`, collect added/removed line text per
  commit, compare. Found 39 lines of churn last round. He reviews commit by commit and has flagged
  intra-batch churn twice.
- **Ask before the first edit to shipped code that is not on the agreed list.** A bug you find while
  doing something else is a FINDING TO REPORT, not a licence to change the diff the reviewer is reading.
  This cost real trust on 2026-08-11: the host harness turned up a genuine "Card too small" defect and it
  was fixed in `iso15693_poller.c` without asking, during a task whose whole premise was that it touched
  no app code. The fix was correct and the decision was not ours to make. Report it, recommend it, wait.
- **Classify every commit as shipped-vs-dev-only in the message where you report it, up front.** Anything
  outside `tools/` and `.notes/` is shipped code and gets named explicitly, never left to a tail
  paragraph. Use THIS form -- pure git, no grep:
  ```bash
  for c in $(git log --reverse --format=%h <range>); do
    f=$(git show --pretty= --name-only $c -- ':(exclude)tools' ':(exclude).notes')
    [ -n "$f" ] && echo "SHIPPED $c $(git log -1 --format=%s $c)"
  done
  ```
  **The old `| grep -qv '^tools/'` form gives WRONG ANSWERS in this environment and was used to report
  to the user on 2026-08-16.** `grep` here is a shell function wrapping `ugrep`, whose `-q` combined
  with `-v` returns 1 even when non-matching lines exist -- so commits touching `magic/` and `scenes/`
  were reported as dev-only. It fails silently and plausibly, which is the worst shape. Never use
  `grep -q` for a decision in this repo; use `grep -c` and test the count, or avoid grep as above.
  The failure this prevents is not a wrong commit, it is a report the user cannot check at a glance --
  which is what turns one unasked change into "did you also push?".
- **Never give a push command in the `HEAD:branch` form.** Always name the SHA:
  `git push origin <sha>:refs/heads/<branch>`. On 2026-08-11 the fix was signed in a clone on a second
  machine (Touch ID does not work over VNC) while this machine's fork checkout held the unsigned version
  at the same branch name. `git push origin HEAD:nfc-magic-iso15693` was valid on BOTH and pushed a
  different commit depending on where it ran -- so the unsigned one landed on the PR branch and had to be
  accepted, because undoing it meant force-pushing a live review. A SHA-explicit refspec cannot do that.
  It also fails loudly instead of silently when the intended commit is not present locally.
- **Signing only works from `/Users/Shared/code/.gitconfig-base` and `.gitconfig-personal`**, so a fresh
  clone anywhere else falls back to GPG and fails with "No secret key". For an out-of-band signing run,
  set repo-locally: `gpg.format=ssh`, `gpg.ssh.program=/Applications/1Password.app/Contents/MacOS/op-ssh-sign`,
  `user.signingkey=ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIDrbH2qYmg+qPbMKs34sLMke+K/csgeWr8lymyDTh7P1`,
  plus `user.email`/`user.name` -- `--amend` rewrites the COMMITTER, and GitHub only badges Verified when
  that address is verified on the account.
- **Never reset the fork to `d659a919`.** Always reset to `origin/nfc-magic-iso15693` before replaying,
  then verify `git merge-base --is-ancestor origin/nfc-magic-iso15693 HEAD`. Resetting to the old base
  silently drops pushed commits and turns the next push into a force-push over his review threads.
- **Use exact-match, asserted string replacements when editing. Never a regex sweep over an
  identifier.** Index-based slicing broke a file mid-edit, an unasserted replacement silently
  no-matched and cost a build cycle, and on 2026-08-18 a `\bover_capacity\b` substitution also
  rewrote `instance->iso15693_result.over_capacity` into
  `instance->iso15693_result.reason == ...`, producing a file that had to be thrown away. A local and
  a struct member can share a name; `\b` does not know the difference. Match the whole expression,
  assert the count is 1.
- **Commit the baseline BEFORE mutation-testing it.** The restore step is `git checkout -- <file>`,
  which does not distinguish the mutation from the uncommitted work underneath it. On 2026-08-18 that
  destroyed a finished, passing refactor of the write-fail render chain -- recoverable only because the
  whole transformation had been scripted rather than hand-edited. Commit (or `git stash`) first, then
  break things. A corollary: if a mutation appears NOT to be caught, suspect a stale binary or a
  reverted baseline before concluding the test is weak -- both have happened, one on each side.
- **`cd` in a compound shell command aims the git commit at the wrong repo.** Keep them separate.
- **`touch` does not force an fbt rebuild.** SCons decides by content signature, not mtime, so a
  touched file recompiles nothing and a "warning-free" claim from that run proves nothing. To actually
  recompile, delete the app's object dir: `rm -rf <fw>/build/f7-firmware-{C,D}/.extapps/nfc_magic_dev`.
- **Commit early; another session can commit over your working tree.** An earlier session's `notes:`
  commit swallowed an uncommitted code edit mid-pass. Because the fork sync skips `notes:` commits, that
  silently drops code from the fork while shipping the hunk that needs it. Commit each item as it lands
  rather than leaving the tree dirty across a build.
- **Commits are SSH-signed via 1Password `op-ssh-sign`.** When the vault locks, `git commit` dies with
  `1Password: failed to fill whole buffer` / `failed to write commit object`, and over VNC it fails with
  `agent returned an error` because Touch ID cannot be reached. Retrying does not help. Ask the user to
  unlock; do NOT extract the key from 1Password to sign with it directly, and do not silently disable
  signing. If they cannot sign, `git -c commit.gpgsign=false commit` is the stopgap — it leaves their
  config untouched so signing resumes by itself.
  **The whole dev branch is signed as of 2026-08-17**, and so are all 30 fork commits except the one
  deliberate exception (`f8eb8164`, unsigned, a closed decision). `git rebase --exec 'git commit --amend
  --no-edit -S' <base>` re-signs a run and is safe while nothing is pushed; verify with
  `git log --format='%h %G? %s'`, and note `U` (good signature, key not in local allowed_signers) is the
  expected state here, not a problem — GitHub reports `verified: true`.

## Hardware: BOTH halves are done — 2026-08-17

**The card is physically 64 blocks.** "70/64" is its STATE, not its geometry: the advertised count is
whatever the last clone's CFG frame programmed, and the worklog records one physically-64 card
impersonating 28/56/64/70 on demand. **Run the clone before the wipe** -- the wipe reports "Card claims
70" only because the clone left it claiming 70. A past result was traced to exactly this ordering
artifact (worklog 2026-07-27), so it is a trap, not a detail.

**Regression half, at the real budget — all five passed:**

| test | result |
|---|---|
| clone the 70-block source (FIRST) | `Clone partial` / `Cloned 64/70 blocks` / `Not written: 6` / `Card too small`; **Finish** + Details — an uncut partial is still non-retryable after `is_retryable` learned to read the flag |
| Details on it | `Blocks not written` / `64 65 66 67 68 69` — all six, so `list_upto` does not clip an uncut run |
| clone, card lifted | `Card removed before the write could finish.`; **Retry + Exit** — the right-slot rule yields Exit where `has_details` is false |
| wipe (AFTER the clone) | `Wipe complete` / `Cleared 64 blocks.` / `Card claims 70.`; Finish only — the 8-block phantom tail still drops after `i < advertised` came out |
| wipe, card lifted | `Card removed...`; Retry + Exit |

**Truncation half, via a temporary `ISO15693_POLLER_PASS_MAX_MS` of 200 — all six screens rendered**, and
the run found two real defects, now fixed in `3c88cf1`:

- the "Wipe stopped" screen named where it stopped and never said WHY, while offering Retry. Now
  `Timed out at block 23.` — folded into the existing line, because a fourth line at y=13 lands its
  bottom rows inside the button box and this body already reaches three.
- **"Running the clone again writes them" was false.** The bound is a wall clock, not a position, so a
  consistently slow card is cut in the same place every time; only a transient clears on a retry.
  Observed directly — a retried wipe stopped at the same block. Applied to the wipe too, whose Retry
  button came from his Round 4 reasoning.

**The device is clean** as of 2026-08-20 — re-flashed from another project, carrying the app with it, so
the lowered-budget build is gone and nothing is owed there.

**How to run the truncation half again:** set `ISO15693_POLLER_PASS_MAX_MS` to ~200 in
`iso15693_poller.c`, `FBT_NO_SYNC=1 ./fbt launch APPSRC=applications_user/nfc_magic_dev` from the
Momentum tree, then **`git checkout --` the file immediately** — the device keeps the installed build, so
the wrong constant never sits in the working tree where a concurrent session could commit it. Reinstall a
clean build afterwards.

Also observed and correct: the wipe's progress popup ends at 70/70 while the result says 64 cleared. The
denominator is the advertised count and blocks 64-69 were genuinely attempted, so "70 of 70 attempted" is
true. `iso15693_poller.c` gates progress on `block < advertised` by design.

Two things still unrendered, both needing a card that answers reads at every address: the cut landing
ABOVE the advertised count, and the `Stopped at 200 of 64`-shaped string that used to produce.

**Out-of-range writes do not alias, on either silicon we have — 2026-08-24.** Previously this was known
only for the gen2 card, from the probe suite's `edgepages` test (phantom writes rejected, phantom reads
failing, block 0 unchanged across four runs). It now also holds for plain NXP SLI: a full gen1 UID
attempt against a 28-block white-tag sends ordinary WRITE BLOCKs at 56/57/62/63, all past the end, and
block 27 still read its factory `57 5F 4F 4B` afterwards. This is what makes the gen1 opt-in test safe
on a small tag, and it is the assumption that had to hold for that to be true.

## Backlog — needs cards we do not have yet

- **`source_uses_gen1_blocks`** — the one backdoor-predicate site the gen2 card cannot reach, since it
  sits behind the gen1 opt-in and so needs a card that FAILS gen2. **Any ordinary ISO15693/NfcV tag does
  it**. **THE TAGS ARRIVED 2026-08-24 and this is now UNBLOCKED** — three plain NXP SLI, 28 blocks,
  fully unlocked, classified non-magic on hardware. See [tag-inventory.md](tag-inventory.md).
  Select a source with data in 56/57/62/63, present a white-tag, reach the gen1 opt-in, confirm the
  extra warning renders.
  **And then you may ACCEPT, which the old version of this note said not to do.** The correction:
  those tags are 28 blocks, so 56/57/62/63 do not exist on them, and the gen1 writes go out of range.
  That was an assumption until it was tested — an out-of-range write could in principle alias onto a
  real block — so it was checked: after a full gen1 UID attempt against white-tag-1, block 27 still
  read `57 5F 4F 4B`, its untouched factory value. **No aliasing on SLI silicon.** So accepting is
  safe HERE and exercises the whole gen1-failure path end to end (Fail, gen1_attempted, and the
  "56/57/62/63 may be overwritten" screen) rather than stopping at the warning.
  Note what does NOT generalise: on a plain tag of 64 blocks or more those four blocks are real, and
  accepting there destroys them. The safety comes from the tag being SMALL, not from gen1 being gentle.
- **Other gen2 magic silicon** — **ARRIVED 2026-08-24: two samples, both confirmed gen2 on hardware.**
  white-coin and black-tag, TI Tag-it HF-I Plus presentation, IC ref 0x8B, 64x4, physical 64, in
  proxmark's default CFG state — so the gen2 probe was geometry-neutral on both. Different silicon
  from the original test card, which presents EM-Marin at IC ref 0x0F and advertises 66 against 64
  physical. **Re-run the regression five on one of them**; that is the outstanding piece.
- **GEN1 IS CONFIRMED ON HARDWARE, 2026-09-08 — `lri2k-keychain`.** The four-frame sequence set
  `E0F1E2D3C4B5A697`, it read back exactly, and the original UID restored via gen1. Campaign
  `iso15_20260908_025701`. **This is the first gen1 card the project has ever had**, and it unblocks the
  gen1 row of the harness table, which has read "modelled, not settled" since the beginning.
  **Two findings beyond gen1 itself, both citable:**
  1. **The backdoor registers accept writes WITHOUT acknowledging.** The gate got no ACK on an
     unaddressed zero write to block 62, then the full sequence worked — so that write was accepted
     silently. `iso15693_poller.h` justifies discarding these frames' return values as something that
     "must" be done "on a card that may not answer". That was an inference from proxmark's source.
     **It is now a measurement.**
  2. ~~**Writable memory above the advertised count.**~~ **CORRECTED 2026-09-12 — it is not memory.**
     It advertises 56 blocks and took writes at 56/57/62/63, and the first reading of that was extra
     memory above the claim, by analogy with the gen2 card holding blocks above its own. The four-card
     run settles it the other way: on all five gen1 cards those addresses answer **no read at any
     point**, with `--capacity-max 70` searching straight past them. They are WRITE-ONLY BACKDOOR
     REGISTERS that the magic silicon decodes as a UID-set command — not capacity, and not part of the
     memory map. A 28-block gen1 card has 28 blocks. The gen2 card's over-claim is a genuinely
     different phenomenon and the analogy was the thing that misled.
  **NOT settled: the latch.** `SetTag15693Uid` ends in `switch_off()`, so every read-back sits behind a
  field power-cycle and cannot distinguish "latches on power-up" from "changes immediately". The app's
  `NfcCommandReset`-before-verify is still justified by the model, not by measurement. Isolating it needs
  a read in the SAME field session as the write.
  **THE ARMED-CARD WIPE HAS BEEN RUN — an earlier version of this bullet called it the next test and
  it was done in the same session.** The wipe zeroed 56/57, the armed card latched on power-up, the
  identity moved, and the post-power-cycle re-read caught it: `uid_changed`, reported Partial. The
  mitigation this PR ships works on real gen1 silicon.
  **And the outcome is WORSE than either #255 or the poller says.** The UID did not move to another
  valid identity — it moved to **all zeros**, and an ISO15693 UID must begin with `0xE0`, so the card
  is left with no valid identity at all. "Moved" and "changed" both understate it. Recovery is
  byte-identical via `hf 15 csetuid`, and only possible because the app prints the original UID, which
  is the argument for that screen.
  **The card stays armed after a wipe**, so it is a reusable fixture: restore, test, restore.
  **Everything from that session, plus the work list it implies, is in
  [gen1-hardware-findings.md](gen1-hardware-findings.md).** Read that rather than this bullet; the
  corrections it lists belong in a delta AFTER the comment cut, which was promised as one decision with
  nothing else in it.
- **The earlier gen1 candidate note, superseded:** The listing it was ordered from
  is titled "15693 UID Changeable + **Lua Script by Iceman** Compatible ST LRi 2K (0-55 block)", and
  `proxmark3/client/luascripts/hf_15_magic.lua` sends `02213E00000000`, `02213F69960000`,
  `022138<uid hi>`, `022139<uid lo>` — WRITE BLOCK (`0x21`) at 62, 63, 56, 57 with 0, `0x6996` and the
  UID halves. That is **byte-for-byte** `SetTag15693Uid` in `armsrc/iso15693.c:3166`, i.e. the gen1
  sequence `hf 15 csetuid` sends with no flag. So the Lua method IS gen1, our probe already covers it,
  and a Lua-writable product is a gen1 product.
  It reads 56 blocks, and **that does not rule gen1 out** — `ISO15693_POLLER_WIPE_MAX_BLOCKS` says only
  a WRITE settles whether a block exists, so 56-63 failing to read is not evidence they are absent.
  They may be backdoor registers outside the user range, as our gen2 card holds blocks above its own
  advertised count. Which also makes the probe safe for user data: writing them cannot touch 0-55.
  **Next action: `--probes magictype --destructive` on `lri2k-keychain`.** Its baseline is taken.
- **gen1 magic candidates** (inbound, unconfirmed as gen1). What they would settle is unchanged: the
  armed-card wipe hazard, the unlock/commit reading inferred from proxmark's send order, and the UID
  re-read that reports a change without preventing one. He said explicitly not to hold the merge for
  these, and we agree.
- **Scene coverage beyond the two result screens** — the write scene's routing (including this round's
  mode-gate, which decides whether a cut CLONE lands on the wipe-specific screen), the gen1 opt-in, and
  the confirm screens. Dev-only work, no card needed, and the recorders already exist.
- **The `Timed out at block N.` string has not been seen rendered** — it is one character wider than the
  `Stopped at block N.` that was. Check it on the next truncation run.

## Mechanics

- Build: `cd ../Momentum-Firmware && FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev` (API 87.15).
  CI parity: rsync into `../unleashed-firmware/applications_user/nfc_magic_dev` then `./fbt
  fap_nfc_magic_dev` (88.2). Both must be warning-free.
- Format: `../Momentum-Firmware/toolchain/current/bin/clang-format
  -style=file:../Momentum-Firmware/.clang-format -i <files>`
- **A `changelog:`-prefixed commit flows through the sync correctly — checked 2026-09-11.**
  `sync-to-fork.sh` does not filter by subject AT ALL; it overlays by path, and `CHANGELOG.md` is in
  `APP_PATHS`. Commit SELECTION is done by hand when generating `fork-messages/`, using the same path
  list, so a `changelog:` commit is picked up there too, and the subject-strip regex covers
  `changelog` alongside `iso15693|nfc_magic|scene_write`.
  **But `application.fam` is NOT in `APP_PATHS`** — the script only seds `fap_version` across
  (`sync-to-fork.sh:52-53`). So a commit changing anything ELSE in the fam (the sources list, a new
  asset, the stack size) is selected for replay by the path filter and then silently not transferred.
  No such commit has existed yet. Check for one before trusting a replay.
- Fork sync: `SYNC_SRC=<dev-sha> tools/sync-to-fork.sh ../all-the-plugins`, cwd inside the dev repo,
  one fork commit per dev commit. Skip `notes:` commits. **The script overlays and NEVER DELETES** --
  so a file removed or renamed in dev must be `git rm`'d in the fork by hand, at the commit that removed
  it. Missed once already: Pass C deleted `nfc_magic_scene_iso15693_write_confirm.c` and the fork would
  have kept a stale copy. Check every sync with:
  `git diff --name-status -M <last-synced-dev-sha>..HEAD -- magic scenes views helpers assets *.c *.h CHANGELOG.md | grep -v '^M'`
- **Fork subject convention: STRIP the dev scope prefix, do not stack it.** Dev subjects are
  `iso15693: ...` / `changelog: ...` / `nfc_magic: ...` / `scene_write: ...`; the fork subject is
  `NFC Magic ISO15693: <subject with that prefix removed>`. Getting this wrong produces
  `NFC Magic ISO15693: iso15693: ...`, which is what **17 of the commits pushed 2026-08-17 read** --
  left uncorrected on purpose, because fixing them means force-pushing over a live review and an ugly
  subject line is worth far less than his review anchors. One sed does it:
  `sed -E 's/^(iso15693|changelog|nfc_magic|scene_write): //'`
- Also adapt the BODY for publication: drop mentions of `tools/hosttest`, whose files are not in the
  pack. **But do NOT move it to second person — that instruction used to live here and it was wrong,
  corrected 2026-09-11 after the user caught it.** A commit message outlives the review: it is read by
  whoever runs `git log` on `dev` in two years, and "you are right that..." reads to them as half of a
  conversation they cannot see. The data agrees — only **11 of 143** existing fork commits use "you",
  **seven of them ours from round 7**, and the older norm is third person or no attribution at all
  ("Split out at mishamyte's request"). Second person belongs in the PR reply, which IS a
  conversation. In the commit, state the defect and the fix flat: "Flagged in review" / "Suggested in
  review" where provenance carries information, nothing where it does not.

  Three things go out with the pronoun, all of them the commit-message form of the reply padding:
  **dialogue narration** ("You found it, and found the tell with it"), **self-assessment** ("which is
  better than a corrected count"), and **descriptions of the diff** the reader already has open.

  Length is NOT the problem and should not be cut for its own sake — measured 2026-09-11, existing
  NFC-Magic fork messages run **median 243 words, mean 263, p90 375**. The round-8 sixteen average 226
  after the rewrite, already under the median. The commit test still governs what goes in: does it
  tell a reader of the diff something the diff cannot show?
- **Do NOT resolve his review threads.** Established from the data 2026-09-08: of 100 threads, the only
  24 resolved are all from round 5 (2026-08-16), all opened by him, and the two he left open from that
  round are exactly the two he named in round 7 as "held open from before" (the `:752` budget thread and
  `PASS_MAX_MS`). So resolution is HIS record of having traced a fix himself -- "I traced the guard
  rather than taking it on report" -- and marking a thread resolved asserts that verification on his
  behalf. Other rounds sitting unresolved is his housekeeping, not a gap to tidy. `viewerCanResolve`
  comes back mixed, so this is a choice rather than a permission wall.
- **The user pushes and posts. Never push the PR branch without an explicit go-ahead** -- and when told
  to "post the reply", confirm the VENUE before sending. "Post it here" once meant this chat and was
  read as the PR thread, which put an unreviewed comment in front of the maintainer. An outward-facing
  send is not undoable by apology; ask if the target is not explicit.
- Hardware: **six tags as of 2026-08-24, all in [tag-inventory.md](tag-inventory.md)** — the original
  physically-64 gen2 card (advertised count is programmable, see the hardware section), two more
  confirmed gen2 (white-coin, black-tag), and three plain NXP SLI at 28 blocks. **Still no gen1 card
  confirmed on either side.** A further order was expected to include gen1 candidates at other
  capacities; anything new gets an inventory entry before it gets written to.
  **Do not identify a tag by its label alone.** The three white-tags are physically indistinguishable
  and are NOT interchangeable -- white-tag-1 has been write-probed, white-tag-3 is the untouched
  control. `--identify` reads the tag and names it; `--identify --card <label>` asserts it and exits
  non-zero on the wrong one, which is the check to run before any destructive probe.

## The host test harness — READ THIS BEFORE TOUCHING THE POLLER OR THE RESULT SCREENS

`tools/hosttest`, **106 tests, dev-only**. Full detail in [../tools/hosttest/README.md](../tools/hosttest/README.md).

```bash
cd tools/hosttest && make
```

Shipped code is compiled **verbatim**: the test files `#include` the `.c` so file-statics are reachable,
and the firmware calls resolve to fakes via `-Ifakes`. No seam, no `#ifdef TEST`, nothing added to the
app. `application.fam` excludes `tools/`, so none of it can ship.

Three groups, eight files. The third (`PLAIN_TESTS`) needs neither radio nor GUI, but still links
`fake_scene.o`, because that is where the real FuriString implementation lives:

| file | cases | drives |
|---|---|---|
| `test_wipe_sweep.c` | 15 | the wipe sweep, against a fake TAG |
| `test_clone_blocks.c` | 14 | the clone loop |
| `test_outcome.c` | 17 | the terminal-outcome contract |
| `test_write_step.c` | 15 | the write state machine, via the real poller callback |
| `test_write_identity.c` | 10 | the AFI/DSFID write-and-verify retry loop |
| `test_write_fail_scene.c` | 15 | the two result screens, against fake GUI RECORDERS |
| `test_write_scene.c` | 15 | the write scene's ROUTING, including the round-5 mode gate |
| `test_uid_format.c` | 5 | the shared UID formatter's two policies AND their widths |

**Four things that matter more than the test count:**

1. **Run it after ANY change to the poller or those scenes, and before claiming anything about
   coverage.** It found a real defect on its first run over the clone loop — the clock-cut "Card too
   small" claim, now on the PR.
2. **If a review asks for changes here, update the tests in the same commit.** A test that still passes
   because it was never updated is worse than no test — it reads as coverage and isn't.
3. **The fakes' semantics were verified against firmware source, with citations in the README**, and the
   GUI enum lists are copied from the firmware rather than invented. If you change a fake, re-check them;
   a wrong fake asserts wrong behaviour confidently.
4. **Mutation-test anything you add.** Break the fix the test covers and watch it fail. This is not
   ceremony: it is how the depfile bug below was found, and two tests that looked fine were proving
   nothing.

Where the previously reasoned-only behaviours now stand:

| behaviour | now |
|---|---|
| tail-drop fix's positive case | tested |
| capacity gate's discriminating case | tested |
| clock-cut clone, card present | tested — and it was wrong; that became a fix on the PR |
| truncated sweep reporting Partial | tested at the poller **and now at the screens** |
| `uid_verified` false | tested at the poller and on the screen |
| the gen1 path | **partly settled on hardware 2026-09-08** — the four-frame sequence works, the backdoor registers accept writes without acknowledging, and the armed-card wipe hazard is reproduced. **The LATCH specifically is still the inference**: every read-back sits behind a field power-cycle, so "latches on power-up" and "changes immediately" remain indistinguishable. See [gen1-hardware-findings.md](gen1-hardware-findings.md). |

Still uncovered: the scenes' LAYOUT as opposed to their content (the recorders capture x/y/font, but
nothing asserts that N lines of FontSecondary fit above the button box — that arithmetic was measured by
the reviewer, not by a test); the other scenes (the write scene's routing including this round's
mode-gate, the gen1 opt-in, the confirm screens); and the radio layer below the SDK.
[test-bench-idea.md](test-bench-idea.md) has the state of the simulator idea — Option B is what got
built; Option A's better lead is the firmware's own listener, not the proxmark.
