# Round 12 — fork messages, and why three sync points need no reordering

Unlike round 11, dev order already matches the fork order: exactly three commits touch shipped files
and each is one decision, in sequence. No reorder, and nothing to fold.

| # | sync at | one decision |
|---|---|---|
| 01 | `a5070a1` | the clock cut decided in one place |
| 02 | `5ea5a5b` | the succeeded-blocks subtractions say why they saturate |
| 03 | `998e4b4` | one clamp for block size |

## What is NOT in these commits, and must not be claimed by them

Every one of the three landed with a test, and all the tests are in `tools/hosttest/`, which
`sync-to-fork.sh` excludes. **A fork commit message that described those tests would be describing a
change absent from its own diff** -- the defect he found in round 11. So the messages cover the source
change only, and the test evidence goes in the reply, framed as the harness rather than as this PR.

Two dev commits are dev-only and have no fork commit at all: the fake tag's latch premise and the
fake-flash removal from `tools/`.

## Verified before drafting

120 tests 0 failed, both firmwares build, clang-format clean, and each of the three mutation-checked
from the committed baseline. `make clean` was used in the mutation loop: a scripted revert can land
in the same second as the object file, and make then treats the target as up to date indefinitely.
