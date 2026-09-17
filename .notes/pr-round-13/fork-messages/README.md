# Round 13 — ten sync points, after the independent review pass

The round now carries three things: the round-12 fixes, the simplification pass (P1-P3), and what
mfcarroll's own review pass found in that work. His corrections are **folded into what they correct**,
so no fork commit shows an error and a later one fixing it.

| # | sync at | one decision | folded in |
|---|---|---|---|
| 01 | `eb367ff` | the clock cut decided in one place | P1 + the duplicate budget |
| 02 | `f117a37` | succeeded-blocks, for three callers | P2 + the third caller |
| 03 | `847822a` | one clamp, only one caller can trip it | P3 + the two-callers split |
| 04 | `edd32ec` | the attempted-count invariant | new |
| 05 | `9286cb4` | the helper's name says it writes | new |
| 06 | `d798bb8` | the gen3 claim and the "not memory" fact | + the capacity clause + "above" |
| 07 | `78ea9ca` | the reach as a closed form | + the third gate and A |
| 08 | `8af8067` | three claims fixed in one place, not their twin | unchanged |
| 09 | `8dc01c5` | "copied exactly" | unchanged |
| 10 | `fbf7af0` | the layout note's x convention | new |

Dev-only, no fork commit: the fake tag's latch premise, the fake-flash removal, and the rebuilt
sweep fixtures.

## Churn: five lines, all comment prose, none in code

Dev history was reordered so each correction sits next to its target, verified by tree hash
(identical before and after), and **the helper name was normalised across the whole range** with a
tree filter so `iso15693_poller_cut_pass_if_expired` is the only spelling that ever exists. Without
that the fork would have watched it introduced as `..._pass_expired` and renamed three commits later.

The five that remain are one doc sentence rewritten by 04 and another by 05 — each by the commit
whose subject is that sentence. **There is no code churn at all.**

Two further foldings were attempted and abandoned: 05 cannot move next to 01, because its hunk
context spans the clamp helper's doc that 03 rewrites. It is a doc-only commit either way.

## What these messages must NOT claim

Every test is in `tools/hosttest/`, which `sync-to-fork.sh` excludes, so a message describing them
would describe a change absent from its own diff. The test evidence is in the reply, framed as the
harness.

`05` must not say "renamed" -- after normalisation its diff is the doc only.

## ⚠️ STILL UNSIGNED

Agreed with mfcarroll: unsigned while he was away, signed before the push. **Re-run
`replay-to-fork.sh` with 1Password unlocked and confirm `10 of 10`.** Do not re-sign the dev commits.

## Verified

123 tests 0 failed, both firmwares build, clang-format clean, final tree byte-identical to the
reviewed state (`f34f7ee`).
