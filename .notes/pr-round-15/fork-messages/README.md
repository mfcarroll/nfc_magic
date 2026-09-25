# Round 15 — four sync points, all behavioural

The addressing round. Unlike every round since the comment cut, **every one of these changes
behaviour** — there is no comment-only commit among them.

| # | sync at | one decision |
|---|---|---|
| 01 | `db3d8ac` | data-block writes carry the card's address |
| 02 | `4c1844b` | the OPTION flag, and the read-back it costs |
| 03 | `844de55` | the gen1 loss claim is gated on there being a loss |
| 04 | `721dd30` | the identity writes are addressed and take the flag |

Dev order matches fork order, so no reorder. **Zero intra-round churn**: the round was rebuilt from
21 commits into 8 precisely so that none of these is corrected by a later one. Verified by tree hash
before and after the rebuild — the content is identical, only its shape changed.

The four notes commits are dev-only and are not sync points.

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

## What these messages must NOT claim

- **No tests.** Every test is in `tools/hosttest/`, which does not sync, so a message citing one
  describes a change absent from its own diff. The mutation results, the host-test counts and the
  fake tag's new block kind all stay out. That evidence belongs in the reply.
- **No dev SHAs**, and no reference to the round having been rebuilt.
- **No bench narrative.** The measurements are stated as results, not as the sequence that found
  them — several of them corrected an earlier reading during this round, and he reads the delta,
  not the search.
