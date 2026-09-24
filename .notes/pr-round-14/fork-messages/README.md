# Round 14 — seven sync points on top of his twelve

The fork is no longer downstream of dev: mishamyte pushed `a27d187d..749f10e6` himself. Dev adopted
all twelve, so the replay base is his head and these seven are what we add.

| # | sync at | one decision |
|---|---|---|
| 01 | `150505a` | the clock cut decided in one place |
| 02 | `28d837c` | one clamp, three callers |
| 03 | `9eb91b9` | succeeded-blocks, three callers |
| 04 | `96f9b85` | the attempted-count invariant |
| 05 | `623a8bc` | the layout note's x convention |
| 06 | `71f9689` | the reach rule's third gate |
| 07 | `868c598` | **BEHAVIOURAL** — the gen1 grant is for one run |

Dev order already matches, so no reorder. The adoption commit is NOT a sync point: its content is
his and already on the fork, so the first sync diffs against it and yields P1 alone.

Dev-only, no fork commit: `furi_crash` in the harness fake, the bench notes, the drafts.

## What these messages must NOT claim

Every test is in `tools/hosttest/`, which `sync-to-fork.sh` excludes, so no message cites one --
including 07, whose four tests and mutation check are real but invisible from the fork. The test
evidence goes in the reply, framed as the harness.

`06` must read as building on his `5c97e16f`, not replacing it. Its gates and arithmetic are his.

`07` is the first behavioural change from our side in this PR, and says so in the same terms he used
for his three: what is bench-confirmed, and what the bound on its reach is.

## Verified before drafting

127 tests 0 failed, both firmwares build, clang-format clean, every dev commit signed. Bench:
his `0c5a7d69` and `62b60e2b` pass, `4d20f06e` not run, our `868c598` confirmed end to end by the
two-card Retry.
