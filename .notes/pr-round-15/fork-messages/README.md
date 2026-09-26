# Round 15 — seven behavioural sync points, then the release notes

The addressing round. Every one of these changes behaviour except 05, which is the release notes.
There is no comment-only commit among them.

| # | sync at | one decision |
|---|---|---|
| 01 | `748ba94` | data-block writes carry the card's address |
| 02 | `c22389e` | the OPTION flag, and the read-back it costs |
| 03 | `71eb1e6` | the gen1 loss claim is gated on there being a loss |
| 04 | `018921a` | the identity writes are addressed and take the flag |
| 05 | `e64c0b2` | a clone that lands in a gen1 card's UID repairs it |
| 06 | `a6a5e9a` | what a clone leaves behind, and what it says about it |
| 07 | `40896a6` | the gen1 registers, addressed and no longer mis-scoped |
| 08 | `881ae53` | the 2.3 release notes for this round |

**ALL RELEASE NOTES ARE ONE COMMIT, 08, and nothing before it touches `CHANGELOG.md`.** Written per
commit they get rewritten by later commits in the same push -- three bullets for what is one fact to
a user and then merged, a bullet reworded twice as the bench widened. A release note is a release
artifact, not a running log, and he reads the delta between rounds. The old CHANGELOG-only sync point
folded into 08 with the rest.

**Dev history WAS reordered, twice and deliberately**: the repair moved ahead of the survey so it
could be its own sync point. See below. 07 collapses six dev commits, and that is the one place this
round does not get one decision per commit; the reason is churn.

The notes commits and the host-test fake are dev-only and are not sync points -- which is why 07's
message cites the cards rather than a test.

## Why 08 comes last, and why it carries two decisions

It is the one frame set 01 left out, and it carries its own measurement: at block 62 the addressed
form is the only one NXP silicon answers, which is a fact about the frames rather than about the
safety argument. Folding it into 01 would put a claim about block 62 inside a commit whose subject is
data blocks, and would hide the re-address seam -- the one thing addressing this sequence costs --
inside a commit that already explains a different re-address for a different reason.

**It ANCHORS AT THE ROUND'S TIP, not at the commit that introduced the addressing**, and that is
what makes it carry the arm-model correction as well. Six shipped dev commits land after the
addressing: the correction itself, and then this round's own self-review fixing text the round had
written. Anchored earlier, 08 would ship "measured on two chips" and an 08 would correct it to
three -- a wrong number and its fix, one sync point apart, which is the churn 02 and 06 were both
shaped to avoid. Zero churn won again, and the message is split under `==` headings so the two
decisions stay separable by a reader.

The arm correction is comment and release notes only; nothing about it changes behaviour. **Verify
before replaying** that no shipped commit sits after 08's anchor -- `replay-to-fork.sh` checks this
at the end, by re-syncing from dev HEAD and diffing, and it is the check that would have caught the
seven sync points reaching only as far as `b312653` while six shipped commits sat above them.

## Why 01 and 02 are separate, and why 02 is not two commits

01 is the #251 safety fix and stands alone: it is correct on every card here and no card needs it to
be writable. 02 is what makes TI Tag-it writable at all, and is separable from 01 — worth keeping
distinct so he can weigh them separately if he wants to.

02 is NOT split into the flag and the read-back, because the flag alone is a broken intermediate: it
makes the card accept the write and simultaneously destroys the acknowledgement, so that commit on
its own is one where a wipe zeroes a card and reports that nothing was cleared. That is the error-
then-fix pairing he has flagged twice, wearing a different hat.

03 folds the caveat wording and the outcome for the same reason in reverse: they are one rule — the
gen1 loss claim is made only where there was a loss — applied at three sites. Split, the first
invites "why was the outcome not fixed in the same breath?"

## Why the repair was lifted OUT, and why the survey and the screens stay together

The repair is the round's most consequential behavioural fix -- it stops a clone writing a file's
bytes into a gen1 card's UID registers -- and it had no visible existence, buried inside a commit
whose subject is what a clone leaves behind. A reviewer scanning subjects would never have found it.

**It was first tried as a split of the existing order and reported impossible: "it does not apply
without the survey in the tree."** That was a fact about a PATCH, not about the code. The repair
references no survey symbol at all -- no `clone_residue_*`, no `clone_survey_top`, no
`survey_above_source`; it is entirely inside the poller while the survey reaches the whole scene
chain. They conflict because they edit neighbouring regions of one file and one struct, which is a
rebase problem. So dev history was reordered instead: repair first, then survey.

The reorder is content-preserving and that was checked rather than assumed -- `git diff` between the
pre-reorder tip and the rebuilt one is EMPTY across every shipped path, and across `tools/` and
`.notes/` as well. The code did not change; only its order did, which is what keeps the hardware
bench standing. The repair also builds and passes the host tests at its own commit, so it is a sync
point a reviewer can actually stop at.

**07 still collapses six**: the survey, the register-as-capacity fix, the notes-page wording, the
comment explaining the repair, and the two size-note corrections. Published one per sync point he
would see the survey introduce a geometry note reading "The card reports 28 blocks and IC ref 01,
not the file's" -- which leads with a number that matched -- and then see it corrected twice. **19 of
the 30 lines the survey adds to the details scene are gone or rewritten by the last of the six** --
re-derived 2026-09-26 by matching every line the survey adds to
`nfc_magic_scene_iso15693_partial_details.c` against that file at 07's anchor. It read 16 when the
range was five commits; the figure moves whenever the range does, so re-derive it rather than quote
it. **Zero churn won**, on the same grounds as 02: a sync point must not show him an error we then
fix.

One line of the repair is restructured at 07 -- `if(!skipping && instance->uid_moved_by_write)`
hoisted into a named variable by the register-as-capacity fix. That is a readability change, not a
correction, which is the benign kind.

## Residual churn — one release-notes line, and it cannot be removed

03 changes when a gen1 clone reports Partial. The release-notes line describing that behaviour —
"a clone that used gen1 reports Partial and flags those blocks" — is corrected at 05, so it is
**stale in the fork tree at 03 and 04**. That is the twin-site defect from round 12, at two commits'
width.

A second, milder one: 01 introduces `iso15693_poller_readdress` taking whatever the inventory
returns, and 06 tightens it to accept only the UID the write implies. Not error-then-fix — 01's
version is correct for what 01 does, and the check needs `predict_uid`, which arrives with the
conversion work in 06 that motivated it. Recorded because it is the same SHAPE as the line above and
a reader comparing 01 against 06 will see it.

The first cannot be closed by reordering: `42d961e` does not apply before `131a59f`, so 05 already
sits at the earliest point it can. Closing it would mean collapsing 03, 04 and 05 into a single sync point,
which costs the 01/02-style separation argued for above. Recorded rather than hidden — round 11 did
the same with its 11 residual lines.

## What these messages must NOT claim

- **No tests.** Every test is in `tools/hosttest/`, which does not sync, so a message citing one
  describes a change absent from its own diff. The mutation results, the host-test counts and the
  fake tag's new block kinds all stay out. That evidence belongs in the reply.
- **No dev SHAs**, and no reference to the round having been rebuilt.
- **No bench narrative.** The measurements are stated as results, not as the sequence that found
  them — several of them corrected an earlier reading during this round, and he reads the delta,
  not the search.
- **`gen-2-card` has no known chip.** Do not call it an EM-Marin; that type line belongs to a
  credential cloned onto it. Four identified chips, plus one card whose silicon was never captured.
