# The C/D cut — measured 2026-09-12, before deciding whether to run it

Goal, as stated by the user: **reduce comment volume**. Constraints and measurements stay in the
code; arguments move to the squash message and the PR thread.

The test being applied is the project's own: a comment answers *"what must I not break when I edit
this line?"* — a CONSTRAINT. Reasoning about *why a decision was taken* is commit/PR material.

## Where the mass is

| file | lines | comment | % | in 10+ line blocks | est. removable |
|---|---:|---:|---:|---:|---:|
| `iso15693_poller.c` | 1756 | 813 | 46% | 517 | **240** |
| `iso15693_poller.h` | 304 | 201 | 66% | 79 | 34 |
| `nfc_magic_scene_write.c` | 605 | 132 | 21% | 67 | 32 |
| `nfc_magic_scene_write_confirm.c` | 137 | 50 | 36% | 43 | 23 |
| `..._iso15693_partial_details.c` | 199 | 94 | 47% | 47 | 20 |
| `..._iso15693_write_fail.c` | 509 | 179 | 35% | 34 | 15 |
| `iso15693_info.c` | 304 | 36 | 11% | 0 | 0 |
| `nfc_magic_app_i.h` | 270 | 45 | 16% | 0 | 0 |
| `..._iso15693_gen1_optin.c` | 107 | 18 | 16% | 0 | 0 |
| `iso15693_info.h` | 32 | 8 | 25% | 0 | 0 |
| **TOTAL** | **4223** | **1576** | **37%** | **787** | **~367** |

**43 comment blocks of 10+ lines hold 787 lines — half of all comment on this surface.** That is
where the argument lives; nothing outside those blocks is worth a pass.

## The three findings that decide the shape of this

**1. It is one file.** `iso15693_poller.c` holds 517 of the 787 block-lines and ~240 of the ~367
estimated cut — **65% of the whole opportunity**. This can be one file's delta rather than a sweep,
which is a far more reviewable diff and matches "one decision per commit".

**2. The header's 66% is a trap.** `iso15693_poller.h` is the highest-ratio file on the surface and
the least compressible: only 79 of its 201 comment lines sit in 10+ blocks, and the rest is
per-field contract — exactly what you least want to delete. Do not go after the header because the
ratio looks bad.

**3. Four files are already tight** — `iso15693_info.c` (11%), `app_i.h`, `gen1_optin.c`,
`iso15693_info.h` have **zero** blocks of 10+. Leave them alone entirely.

## The ratio will barely move, and that is expected

1576 → ~1209 comment lines takes the surface from **37% to ~31%**, because removing comment shrinks
numerator and denominator together. This is already recorded: the round-7 cut removed 102 lines and
moved the ratio 37% → 36%.

**So do not report this as a ratio.** The honest figure is **~367 lines out, ~23% of the comment**,
which is nearly 4x the round-7 cut (95 lines).

## Worked examples — what "argument" looks like here

`ISO15693_POLLER_PASS_MAX_MS`, **48 lines**. Constraint: it is a backstop not a tuning knob; the two
errors are asymmetric; both passes share it so the value must suit the more expensive one; the
figures are not a property of the code. That is ~18 lines. The other ~30 are a cost comparison
between the wipe and the clone — frame ratios, which pass pays more, `furi_delay_ms` shares — shown
working that establishes a conclusion the first 18 lines can simply state.

The tail-drop, **42 lines**. The prefix-property paragraph is load-bearing and stays. The 11-line
reconciliation of "why this loop may disagree with `wipe_note_present`" is argument — though it
does guard against a maintainer "fixing" an apparent contradiction, so it compresses rather than
goes.

`ISO15693_POLLER_WIPE_ABSENT_RUN`, **32 lines**. Constraint: sole guard against a dropout reading as
the card's top, set for that not for speed, and the reason not to raise it. ~14 lines. The
coupling-wobble narrative and the per-block cost derivation are argument.

The Back-swallow block, **42 lines**, is the most defensible large block on the surface — most of it
answers "do not naively extend this to the other protocols", which is a constraint. Its 10 lines of
gen2/Classic/gen4 forensics compress to a sentence plus the #252/#253 references.

## Confidence

**9 of 43 blocks were read and classified in full** (~222 of 787 lines, 28%), chosen as the largest
plus three small ones to check the rate at both ends. Reduction rates by size band come from those:
≥30 lines ~55%, 20-29 ~50%, 15-19 ~45%, 10-14 ~38%.

Smaller blocks are denser in constraint, which the sampling reflects. If the 34 unread blocks are
more constraint-dense than the sampled ones, the real figure lands nearer **250-300** than 367.
Treat ~367 as the optimistic end of a 250-370 range.

## Recommended shape, if it runs

1. **`iso15693_poller.c` alone, as one delta.** 65% of the value, one file, reviewable.
2. Leave the header, the four tight files, and anything under 10 lines.
3. Every block cut must have its argument land somewhere — the squash message is the destination,
   which means **the squash message has to be written before or with this**, not after.
4. Verify with `tools/comment-only.py` that code bytes are unchanged. That is the whole safety
   property of a deletion pass, and it is mechanically checkable.

## The risk this pass does NOT carry, and the one it does

Deleting argument cannot make a constraint wrong, which is why this is safer than a rewording pass.

The one real hazard is round 9's thread 09: **a true fact removed alongside a false one**, where
nothing in the result looks wrong afterwards so nothing prompts a re-check. The guard is to read
what a paragraph does BESIDES make its argument before cutting it.
