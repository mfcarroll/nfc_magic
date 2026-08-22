# The comment cut — starting point for Round 7

Committed to in writing at the end of the Round 6 reply.

**Option 1, the ownership model, is the intended path** — decided on our side 2026-08-20, independently of
his answer. Still read his reply first: if he asks for the narrow version instead, that governs, and
option 2 is recorded below for that case. But do not treat the choice as open by default.

Do it as **its own delta with nothing else in it**, so the diff reads as one decision. That was promised.

## Why, in one paragraph

Round 6 was eleven threads of comments contradicting the code, three of them contradicting *other comments
in the same delta*. Fixing them cost **+117 comment against +28 code** and took `iso15693_poller.c` from
43% to 44%. The problem is not density, it is **duplication**: a fact stated in three places drifts in two,
and every correcting round makes the ratio worse. The cut is the fix for the mechanism, not for the
percentage.

## Where the mass actually is — measured 2026-08-20

| file | lines | comment | % |
|---|---|---|---|
| `iso15693_poller.c` | 1700 | 756 | **44%** |
| `iso15693_poller.h` | 287 | 185 | **64%** |
| `nfc_magic_scene_iso15693_partial_details.c` | 192 | 90 | 46% |
| `nfc_magic_scene_iso15693_write_fail.c` | 500 | 170 | 34% |
| `nfc_magic_scene_write.c` | 604 | 131 | 21% |
| `nfc_magic_app_i.h` | 259 | 41 | 15% |
| `gen1_optin.c` | 108 | 19 | 17% |
| `file_select.c` | 134 | 19 | 14% |
| **ISO15693 surface** | **3784** | **1411** | **37%** |

`iso15693_poller.h` at **64%** is the surprise and probably the place to start: it is a header, so every
duplicated sentence there is duplicated *away from* the code that implements it, which is the drift
mechanism in its purest form. `gen2_poller.c` and `uscuid_ul_poller.c` sit at ~8% for comparison.

## Option 1 — the ownership model (THE PLAN)

One owner per fact. Everything else cross-references instead of restating.

| fact | owner |
|---|---|
| the outcome contract (what Success / Partial / Fail mean) | the `Iso15693PollerEvent` enum |
| the wire facts (which blocks are which register, per generation) | the `ISO15693_MAGIC_BLK_*` defines |
| the user-facing gen1 consequence | `gen1_optin.c`'s strings |
| the run-budget rationale and its arithmetic | `ISO15693_POLLER_PASS_MAX_MS` |
| the activation cache's prefix property | the read-back note in the sweep |
| the line-budget arithmetic (what fits above the button box) | one place in `write_fail.c` |

The known duplicated facts, from six rounds of his findings — these are the concrete work list:

- **the back-fill's scope** (clone-only) — was wrong in three places in Round 6
- **the two truncation archetypes** (which card gives which gap) — was fused in four places
- **the prefix property** vs **`pvPortMalloc`'s memset** — two independent facts, repeatedly conflated
- **the cost figures** under the budget — corrected twice, and the conclusion inverted the second time
- **`blocks_total` semantics** (a count, not an index) — restated in at least three places
- **the right-slot button rule** — stated in two functions, was two-way in one of them
- **the gen1 56/57/62/63 hazard** — appears in the poller, `app_i.h`, the CHANGELOG and now #255

## Option 2 — narrow (fallback, only if he asks for it)

Only the seven facts above, left where they are, corrected and cross-referenced. Cheaper, and leaves the
mechanism intact — which is why it is the fallback rather than the plan.

## Also in this pass, per his Round 6 notes

- **the compact-UID formatter's four copies**: `iso15693_info.c:18`, `write_fail.c:287`, `:305`,
  `write_confirm.c:39`. He was explicit that the fold *relocated* one rather than adding one, so this is
  not a charge against the Round 6 delta.
- **the twelve `widget_add_string_multiline_element` calls** varying only in `(x, y)`. Deliberately left in
  Round 6: those y values carry the line-budget arithmetic he measured for us in Round 4, and burying them
  in a table would lose it. They move here, with that arithmetic given one owner.

## How to do it safely

**The tests exist for exactly this.** 101 cases, and the two scene files cover the result screens and the
write scene's routing, so a comment-only pass is *verifiable* as behaviour-preserving instead of
read-and-hoped. That was the point of building them before this round.

1. Run `cd tools/hosttest && make` first and note the count. Any change in it is a behaviour change.
2. Work file by file, comment-only where possible. If a comment cannot be cut without moving code, that is
   a separate commit.
3. Re-run the tests after each file, and both firmware builds at the end.
4. **Report comment-vs-code per commit.** The target is strongly negative comment with code at zero. A
   commit that adds comment is going the wrong way and needs saying so.
5. Churn audit before pushing — he has flagged intra-batch churn twice.

## What NOT to cut

The rules that earned their place, which he has never objected to:

- constraints that would make a wrong refactor look attractive — tick wraparound, don't-reuse-`is_wiping`,
  the widget copying its string so the early free is safe
- the measured hardware figures and the disassembly facts, which cannot be re-derived from the code
- the "Not closed" dispositions, which record a decision rather than an omission
- anything a test now enforces should point AT the test rather than re-arguing itself

## Do not re-open

Settled in Round 6 and listed in the brief: the capacity guard and 10s, the render table's shape
(`{reason -> title}`, done), `WipeUidChanged` withholding Retry, and `NothingWiped` having no
`uid_verified` route.

---

# EXECUTED 2026-08-22 — eight commits, local only, NOT pushed

Option 1 as planned. mishamyte had not replied when this was done, so the stated preference governed.
Dev `iso15693-dev`, eight commits on top of `04d5f8a`, all signed, **zero intra-batch churn**.

| commit | comment | code | |
|---|---|---|---|
| `3199bb9` | −27 | +0 | the header states each fact once |
| `dc6d749` | −37 | +0 | the private struct stops re-documenting the public one |
| `58de6ba` | −10 | +0 | the constants stop deriving each other's figures |
| `9ca8201` | −10 | +0 | #255 owns the gen1 hazard argument |
| `99eae6f` | −1 | +0 | three facts inside the poller that had two homes each |
| `805a716` | −15 | +0 | the scenes stop restating the poller's contract |
| `16f5c14` | +5 | **+11** | one UID formatter for the four screens that print one |
| `4f47bc0` | +0 | +0 | give the retry-is-not-a-promise fact an owner |
| **total** | **−95** | **+11** | |

Seven of the eight are comment-only, **proven** rather than asserted: with comments stripped each file is
byte-identical, and the host tests read exactly 101 before and after every one of them.

## The finding that matters more than the delta

**The ratio is the wrong metric and it should be said out loud.** On the plan's own eight files:

| | lines | comment | % |
|---|---|---|---|
| before | 3776 | 1411 | 37% |
| after | 3672 | 1309 | **36%** |

−102 comment lines moved the ratio **one point**, because removing comment lowers the numerator and the
denominator together. Chasing 8% by deduplication is arithmetically impossible: `iso15693_poller.c` would
need to drop from 698 comment lines to about 236, and what is left after this pass is the hardware
measurements, the view_dispatcher deadlock argument, the read-back classification table and the
don't-de-arm constraint — documentation, not duplication.

**The metric that does track the defect** is how many places state the same fact. Counting repeated
6-word comment phrases across the nine ISO15693 files:

- **before: 135**
- **after: 34** — and the residue is parallel structure in the header (the two gen1 entry points),
  cross-references naming the same owner twice, and ordinary English like "contiguous run at the top of
  the card".

Sites per fact, before → after: `56/57/62/63` **10 → 1**, "first activation" **6 → 1**, the gen2 CFG
frame **2 → 1**.

For honesty: `gen2_poller.c` is 875 lines at 9% and `uscuid_ul_poller.c` 515 at 8%, so the gap is not a
size artefact — ISO15693 carries roughly 9x the comment per line of code. That is a real difference and
worth conceding as one. The defensible claim is about WHAT the residue is, not that the gap is imaginary.

## What was found along the way

- **The private struct was the single biggest duplication in the PR.** `struct Iso15693Poller` carries a
  reporting field for all but one member of `Iso15693PollerResult`, and every one had its own paragraph
  restating the header's. `get_result` is already the mapping — one assignment per field — so the prose
  mapping was redundant with code. Two parallel doc sets is the Round 6 mechanism in its purest form.
- **Two duplicates were inside a single comment.** `start_clone_gen1` said "the gen1 registers now hold
  the UID, so they cannot match the source" twice in one block.
- **An eighth duplicated fact, not on the list above**: the header explained why
  `has_details(WipeStopped)` is unconditional, which the scene already says at its own `has_details`.
- **A drift the cut exposed**: `ISO15693_POLLER_WRITE_ATTEMPTS` said "a failed *clone* WRITE BLOCK" while
  both passes use it — contradicted by the first line of the function it governs.
- **The retry-is-not-a-promise fact had no owner at all.** The hardware-derived finding from Round 5
  (a retried wipe stops at the same block) was independently DERIVED in `is_retryable` and in the
  Details note. It now belongs to `pass_truncated`.

## The one code change, and the one thing deliberately not done

`16f5c14` folds four hand-rolled UID loops into `iso15693_info_cat_uid`. The interesting part: three of
the four implemented the *same* two-group layout in two different spellings (`i == 4` before the byte,
`i == 3` after it), and the reason for it — 23 characters against 17 on a 128px line — was written at
one of the four. It costs **+11 code**, reported not dressed up. What justifies it is that the format is
now TESTED: `test_uid_format.c`, five cases, **mutation-verified three ways** (move the group break →
3 fail; drop the zero-padding → 5 fail; drop the leading-separator guard → 3 fail). Host tests 101 → 106.

**NOT done: folding the twelve `widget_add_string_multiline_element` calls into a table.** Layout is the
one thing the host tests do not cover, so that change would be unverifiable here — and it is a code
change in a delta that promised to be a comment cut. The line-budget arithmetic it depends on now has
one owner at the top of `write_fail.c`, which is the part that was actually duplicated.

## Verification

Momentum 87.15 and Unleashed 88.2, both from a deleted object dir, **zero warnings** from the app.
`clang-format` clean. 106 host tests green. Churn audit across all eight commits: **zero**.

Note on width: `ReflowComments: false` means clang-format never rewraps comments, so the file's de-facto
comment width is 106, not the 99 `ColumnLimit`. New lines were held at 106 rather than rewrapping the
file to 99 — that would have moved every line and buried the cut in churn.
