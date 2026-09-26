# Round 15 — six sync points, all behavioural but one

The addressing round. Every one of these changes behaviour except 05, which is the release notes.
There is no comment-only commit among them.

| # | sync at | one decision |
|---|---|---|
| 01 | `32f80be` | data-block writes carry the card's address |
| 02 | `3327ad3` | the OPTION flag, and the read-back it costs |
| 03 | `0fd69e8` | the gen1 loss claim is gated on there being a loss |
| 04 | `f1a6ae4` | the identity writes are addressed and take the flag |
| 05 | `b9be432` | the release notes overstate what a gen1 clone reproduces |
| 06 | `39df365` | what a clone leaves behind, and a gen1 card it lands in |

Dev order matches fork order, so no reorder. **06 collapses seven dev commits** — see below, because
that is the one place this round does not get one decision per commit, and the reason is churn.

The twelve notes commits are dev-only and are not sync points.

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

## Why 06 carries two decisions

It syncs the tree at the last of seven dev commits: the clone survey, the gen1-card-on-the-gen2-path
repair, the register-as-capacity fix, the notes-page wording, the comment recording why the repair
reacts to the registers rather than predicting them -- which documents code the same sync point
introduces, so it cannot be split off without describing something absent from its own diff -- and
the bench's two corrections to the size note's wording. That last one is why the sync point moved rather
than a seventh being added: appending to the tail of an already-collapsed range costs no churn,
while a commit after it would show him the superseded wording and then its fix. Published one per sync point, he
would see the survey introduce a geometry note reading "The card reports 28 blocks and IC ref 01,
not the file's" — which leads with a number that matched — and then see it corrected twice. **16 of
the 30 lines the survey adds to the details scene are rewritten by the last of the four.**

Splitting the repair out from the survey was tried and is not available: it does not apply without
the survey in the tree, and a tree-based sync cannot take half a commit. So the choice was two
decisions in one diff against three sync points showing him a wrong screen string and its fixes.
**Zero churn won**, on the same grounds as 02: a sync point must not show him an error we then fix.

The message is split under three headings so the two decisions are still separable by a reader.

## Residual churn — one release-notes line, and it cannot be removed

03 changes when a gen1 clone reports Partial. The release-notes line describing that behaviour —
"a clone that used gen1 reports Partial and flags those blocks" — is corrected at 05, so it is
**stale in the fork tree at 03 and 04**. That is the twin-site defect from round 12, at two commits'
width.

It cannot be closed by reordering: `b9be432` does not apply before `f1a6ae4`, so 05 already sits at
the earliest point it can. Closing it would mean collapsing 03, 04 and 05 into a single sync point,
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
