# Round 13 — seven sync points, no reorder, zero churn

Dev order already matches fork order and every shipped commit is one decision, so unlike round 11
there is nothing to reorder and nothing to fold.

| # | sync at | one decision |
|---|---|---|
| 01 | `a5070a1` | P1 — the clock cut decided in one place |
| 02 | `5ea5a5b` | P2 — the succeeded-blocks subtractions say why they saturate |
| 03 | `998e4b4` | P3 — one clamp for block size |
| 04 | `3b7f922` | the gen3 claim and the "not memory" fact |
| 05 | `27f0ec1` | the sweep's reach as a closed form |
| 06 | `7726e74` | three claims corrected in one place and not their twin |
| 07 | `bff4e08` | "copied exactly" |

**Measured, not assumed: zero lines are added in one commit and removed in a later one.** Round 11
had eleven. The reason is that the round-12 fixes and P1-P3 land in different passages, and the
P-items were built before the review arrived rather than on top of it.

## What these messages must NOT claim

Each of P1-P3 landed with a test, and every test is in `tools/hosttest/`, which `sync-to-fork.sh`
excludes. **A fork message describing those tests would describe a change absent from its own diff**
-- the defect he found in round 11. The messages cover the source change only; the test evidence is
in the reply, framed as the harness rather than as this PR.

Dev-only, no fork commit at all: the fake tag's latch premise and the fake-flash removal from
`tools/`.

## ⚠️ THE DEV COMMITS IN THIS ROUND ARE UNSIGNED

1Password was locked and unavailable while mfcarroll was away, and `git commit` fails opaquely in
that state. The four round-12-fix commits carry a note saying so.

This does **not** affect the fork: `replay-to-fork.sh` creates its own commits and signs them at
replay time, so re-running it with 1Password unlocked produces a fully signed chain. Re-signing the
dev commits would mean rewriting history and renaming every `NN-<sha>.msg` here, which is not worth
it -- but the replay must be run with signing available, and the result checked for `7 of 7`.

## Verified before drafting

120 tests 0 failed, both firmwares build, clang-format clean. The whole round is comment-only
outside the CHANGELOG **except** P1-P3, which are the first code changes since the comment cut and
are each mutation-checked from the committed baseline.
