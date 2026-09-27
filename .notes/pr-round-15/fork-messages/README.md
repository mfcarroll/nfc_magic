# Round 15 — seven behavioural sync points, then the gen2 measurement and the release notes

The addressing round. 01-07 each change behaviour. 08 changes none: it is the whole round's release
notes plus the one comment that had to move with them, because a note stopped arguing a case and
started stating a measurement.

| # | sync at | one decision |
|---|---|---|
| 01 | `748ba94` | data-block writes carry the card's address |
| 02 | `c22389e` | the OPTION flag, and the read-back it costs |
| 03 | `71eb1e6` | the gen1 loss claim is gated on there being a loss |
| 04 | `018921a` | the identity writes are addressed and take the flag |
| 05 | `e64c0b2` | a clone that lands in a gen1 card's UID repairs it |
| 06 | `a6a5e9a` | what a clone leaves behind, and what it says about it |
| 07 | `dcee20c` | the gen1 registers, addressed and no longer mis-scoped |
| 08 | `f689965` | the gen2 frames cannot be addressed, and the 2.3 release notes |

**ALL RELEASE NOTES ARE ONE COMMIT, 08, and nothing before it touches `CHANGELOG.md`.** Written per
commit they get rewritten by later commits in the same push -- three bullets for what is one fact to
a user and then merged, a bullet reworded twice as the bench widened. A release note is a release
artifact, not a running log, and he reads the delta between rounds. The old CHANGELOG-only sync point
folded into 08 with the rest.

**Dev history WAS reordered, twice and deliberately**: the repair moved ahead of the survey so it
could be its own sync point. See below. 06 collapses six dev commits and 07 collapses seven, and
those are the two places this round does not get one decision per commit; the reason is churn.

The notes commits and the host-test fake are dev-only and are not sync points -- which is why 07's
message cites the cards rather than a test.

## Why 07 carries two decisions, and anchors where it does

It is the one frame set 01 left out, and it carries its own measurement: at block 62 the addressed
form is the only one NXP silicon answers, which is a fact about the frames rather than about the
safety argument. Folding it into 01 would put a claim about block 62 inside a commit whose subject is
data blocks, and would hide the re-address seam -- the one thing addressing this sequence costs --
inside a commit that already explains a different re-address for a different reason.

**It ANCHORS ABOVE THE SELF-REVIEW, not at the commit that introduced the addressing**, and that is
what makes it carry the arm-model correction as well. Six shipped dev commits land after the
addressing: the correction itself, and then this round's own self-review fixing text the round had
written. Anchored earlier, 07 would ship "measured on two chips" and a later sync point would
correct it to three -- a wrong number and its fix, one sync point apart, which is the churn 02 and
06 were both shaped to avoid. Zero churn won again, and the message is split under `==` headings so
the two decisions stay separable by a reader.

The arm correction is comment and release notes only; nothing about it changes behaviour. **Verify
before replaying** that no shipped commit sits after 08's anchor -- `replay-to-fork.sh` checks this
up front now, and the end-state diff catches it too. It is what would have caught the sync points
once reaching only as far as the addressing commit while six shipped commits sat above them.

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

**06 COLLAPSES SIX**: the survey, the register-as-capacity fix, the notes-page wording, the comment
explaining the repair, and the two size-note corrections. Published one per sync point he would see
the survey introduce a geometry note reading "The card reports 28 blocks and IC ref 01, not the
file's" -- which leads with a number that matched -- and then see it corrected twice. **19 of the 30
lines the survey adds to the details scene are gone or rewritten by the last of the six** --
re-derived 2026-09-26 by matching every line the survey adds to
`nfc_magic_scene_iso15693_partial_details.c` against that file at 06's anchor, counting a duplicated
line once for each time it was added. It read 16 when the range was five commits; the figure moves
whenever the range does, so re-derive it rather than quote it -- and match multiset, not membership,
or repeated lines like a lone brace read as survivors and the figure comes out low. **Zero churn
won**, on the same grounds as 02: a sync point must not show him an error we then fix.

**07 collapses seven**, for the reason in its own section above.

One line of the repair is restructured at 06 -- `if(!skipping && instance->uid_moved_by_write)`
hoisted into a named variable by the register-as-capacity fix. That is a readability change, not a
correction, which is the benign kind.

## Residual churn — the release-notes line is GONE, and one helper remains

**The release-notes churn this section used to describe no longer exists.** 03 changes when a gen1
clone reports Partial; the notes line describing that behaviour was written at 03 and corrected at
05, so it stood stale in the fork tree at 03 and 04 -- the twin-site defect from round 12, at two
commits' width. Moving every release note into 08 closed it outright: only 08 touches `CHANGELOG.md`,
so no sync point before it can carry a notes line for a later one to rewrite. **Check that rather
than believing it** -- `git log <round base>..<tip> -- CHANGELOG.md` must name exactly one commit.

What remains is legitimate: 01 introduces `iso15693_poller_readdress` taking whatever the inventory
returns, and **05** tightens it to accept only the UID the write implies. Not error-then-fix -- 01's
version is correct for what 01 does, and the check needs `predict_uid`, which arrives with the repair
at 05 that motivated it. A reader comparing 01 against 05 will see it, which is why it is written
down. The other is `send_backdoor_uid_gen1`'s signature changing at 07, for the same kind of reason.

The two clusters that COULD be removed are measured and deliberately left; they are in
[NEXT-SESSION.md](../../NEXT-SESSION.md) under KNOWN REMAINING WORK, at 28 lines (02 -> 04) and 13
(03 -> 06). Round 11 recorded its 11 residual lines the same way rather than hiding them.

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
