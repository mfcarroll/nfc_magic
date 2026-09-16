# The C/D cut — re-measured 2026-09-13 against HEAD

Goal, as stated by the user: **reduce comment volume**. Constraints and measurements stay in the
code; arguments move to the squash message and the PR thread.

The test being applied is the project's own: a comment answers *"what must I not break when I edit
this line?"* — a CONSTRAINT. Reasoning about *why a decision was taken* is commit/PR material.

Reproduce any figure here with:

```bash
python3 tools/comment-ratio.py blocks --list <the ten files below>
```

## The first measurement was of a tree that never shipped

Taken 2026-09-12, mid-round-10, **before** the round was rebuilt from 27 commits to 9. The rebuild
dropped the churn, so the tree it measured does not exist in history. Its table read
**4223 / 1576 / 787 in 43 blocks**; nothing ever had those numbers. Kept here because the figures
were quoted to the maintainer in the round-10 reply and need correcting, not quietly replacing.

| tree | lines | comment | % | in 10+ blocks | blocks |
|---|---:|---:|---:|---:|---:|
| pre-round-10 (`47b1dc3~1`) | 4092 | 1468 | 36% | 688 | 38 |
| the 2026-09-12 note | 4223 | 1576 | 37% | 787 | 43 |
| **HEAD (`6b6aa84` onward)** | **4181** | **1550** | **37%** | **740** | **40** |

Round 10's real cost on this surface: **+89 lines, +82 comment, +52 block-lines, +2 blocks.**

## Where the mass is, now

| file | lines | comment | % | in 10+ blocks | blocks | est. removable |
|---|---:|---:|---:|---:|---:|---:|
| `iso15693_poller.c` | 1758 | 821 | 47% | 503 | 27 | **237** |
| `iso15693_poller.h` | 301 | 199 | 66% | 78 | 5 | 35 |
| `nfc_magic_scene_write.c` | 603 | 131 | 22% | 67 | 3 | 33 |
| `nfc_magic_scene_write_confirm.c` | 128 | 42 | 33% | 35 | 1 | 19 |
| `..._iso15693_write_fail.c` | 503 | 174 | 35% | 34 | 2 | 15 |
| `..._iso15693_partial_details.c` | 183 | 79 | 43% | 23 | 2 | 9 |
| `iso15693_info.c` | 303 | 36 | 12% | 0 | 0 | 0 |
| `nfc_magic_app_i.h` | 266 | 42 | 16% | 0 | 0 | 0 |
| `..._iso15693_gen1_optin.c` | 105 | 18 | 17% | 0 | 0 | 0 |
| `iso15693_info.h` | 31 | 8 | 26% | 0 | 0 | 0 |
| **TOTAL** | **4181** | **1550** | **37%** | **740** | **40** | **~348** |

**40 comment blocks of 10+ lines hold 740 lines — 48% of all comment on this surface.** That is
where the argument lives; nothing outside those blocks is worth a pass.

## The three findings all survive the re-measure

**1. It is one file.** `iso15693_poller.c` holds 503 of the 740 block-lines and ~237 of the ~348
estimated cut — **68% of the whole opportunity**, up from 66%. This can be one file's delta rather
than a sweep, which is a far more reviewable diff and matches "one decision per commit".

**2. The header's 66% is a trap.** `iso15693_poller.h` is the highest-ratio file on the surface and
the least compressible: only 78 of its 199 comment lines sit in 10+ blocks, and the rest is
per-field contract — exactly what you least want to delete. Do not go after the header because the
ratio looks bad.

**3. Four files are already tight** — `iso15693_info.c` (12%), `app_i.h`, `gen1_optin.c`,
`iso15693_info.h` have **zero** blocks of 10+. Leave them alone entirely.

## The ratio will barely move, and that is expected

1550 → ~1202 comment lines takes the surface from **37% to ~31%**, because removing comment shrinks
numerator and denominator together. This is already recorded: the round-7 cut removed 102 lines and
moved the ratio 37% → 36%.

**So do not report this as a ratio.** The honest figure is **~348 lines out, ~22% of the comment**,
which is nearly 3.5x the round-7 cut (95 lines).

## Worked examples — what "argument" looks like here

All four still exist at exactly the sizes quoted, and all four are in the top band.

`ISO15693_POLLER_PASS_MAX_MS` (`iso15693_poller.c:181`), **48 lines**. Constraint: it is a backstop
not a tuning knob; the two errors are asymmetric; both passes share it so the value must suit the
more expensive one; the figures are not a property of the code. That is ~18 lines. The other ~30 are
a cost comparison between the wipe and the clone — frame ratios, which pass pays more,
`furi_delay_ms` shares — shown working that establishes a conclusion the first 18 lines can state.

The tail-drop (`iso15693_poller.c:1084`), **42 lines**. The prefix-property paragraph is load-bearing
and stays. The 11-line reconciliation of "why this loop may disagree with `wipe_note_present`" is
argument — though it does guard against a maintainer "fixing" an apparent contradiction, so it
compresses rather than goes.

`ISO15693_POLLER_WIPE_ABSENT_RUN` (`iso15693_poller.c:231`), **32 lines**. Constraint: sole guard
against a dropout reading as the card's top, set for that not for speed, and the reason not to raise
it. ~14 lines. The coupling-wobble narrative and the per-block cost derivation are argument.

The Back-swallow block (`nfc_magic_scene_write.c:522`), **42 lines**, is the most defensible large
block on the surface — most of it answers "do not naively extend this to the other protocols", which
is a constraint. Its 10 lines of gen2/Classic/gen4 forensics compress to a sentence plus the
#252/#253 references.

## Confidence

Reduction rates by size band come from the 2026-09-12 pass, where **9 of the then-43 blocks were
read and classified in full** (~222 lines, 28% of the mass), chosen as the largest plus three small
ones to check the rate at both ends: ≥30 lines ~55%, 20-29 ~50%, 15-19 ~45%, 10-14 ~38%. Applied to
the current distribution — 5 blocks ≥30, 9 at 20-29, 7 at 15-19, 19 at 10-14 — those give 348.

Smaller blocks are denser in constraint, which the sampling reflects. If the unread blocks are more
constraint-dense than the sampled ones, the real figure lands nearer **240-300** than 348. Treat
~348 as the optimistic end of a **240-350** range.

## What has to be said to the maintainer

The reply quoted **"1,576 comment lines on this surface, 787 of them in 43 blocks of ten lines or
more, and roughly 250–370 removable"**. Every one of those is from the unshipped tree. The true
figures are **1,544 / 740 / 40 / ~240-350** -- re-measured against bd50bf5 itself, which is the
tree he has; an earlier pass here said 1,550 and was six lines out. Small, but it is a measurement stated as a measurement,
and the correction costs a line in whatever goes out next.
