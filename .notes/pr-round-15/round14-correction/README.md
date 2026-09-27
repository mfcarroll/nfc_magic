# Round 14, as it was meant to be pushed — the correction set

Round 14 was pushed from anchors a same-day rewrite had already replaced, so the PR got an older
draft of three comments; see [../final-review.md](../final-review.md) F1. This directory rebuilds
our seven round-14 commits from the commits dev actually carries, and adds the eighth that never had
a sync point. [../../pr-round-14/fork-messages/](../../pr-round-14/fork-messages/) stays as it is:
it is the record of what WAS pushed, anchors and all.

| # | sync at | one decision |
|---|---|---|
| 01 | `986af3b` | the clock cut decided in one place |
| 02 | `9fd5630` | one clamp for block size, across all three callers |
| 03 | `f7b0bf8` | the succeeded-blocks figure is one subtraction |
| 04 | `dd9248f` | the sweep's attempted-count invariant, stated once |
| 05 | `aef86f2` | the write-fail layout note documents its y values and not its x |
| 06 | `280dc42` | the reach rule's third gate |
| 07 | `7c2f07a` | **BEHAVIOURAL** — the gen1 grant is for one run |
| 08 | `f899c78` | four comments state the rule rather than its history |

**Replayed with `BASE=749f10e6 PARTIAL=1`**: onto mishamyte's head, so his twelve are untouched,
and partial because round 15's shipped commits sit above 08 on dev. The replay's base check holds
it to the right starting point — dev's tree before 01 is byte-identical to `749f10e6`'s.

**What changes on the PR, and only this**: three comments shortened in 01, 02 and 07 -- the
clamp's with its miscount corrected and its "matters most" kept; the budget parameter's reason, in
01's comment and message, which pointed at #253 when the idea is the KNOWN OVERLAP note's; one
imprecise sentence in 01's message; and 08's four comments. All comment-only, proven with the
preprocessor. Messages 02-07 are byte-identical to what was pushed.

**The dates**: each `.date` is the author date the pushed commit had, so the round still dates to
2026-09-24. 08 never existed on the fork, and takes the next second after 07 so the round stays in
order; its change was authored on dev at 16:19 the same day. Committer dates are the rebuild's own.
