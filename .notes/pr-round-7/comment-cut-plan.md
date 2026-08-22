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
