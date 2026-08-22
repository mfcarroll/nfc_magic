Round 6, at `64326417`. **All three blocking items are fixed**, and I traced each rather than taking the report for it.

One new blocking item, and it is your own fix half-applied rather than anything new: `success_or_partial`'s Fail guard runs ahead of the Partial test and never reads `pass_truncated`, so a cut clone that accepted nothing is told the card refused every block. Thread on `iso15693_poller.c:1131`.

Resolving the round-5 threads that are settled. Two stay open deliberately: the budget one, because you asked me a question in it and it deserves an answer rather than a checkmark, and the `PASS_MAX_MS` one, because the replacement comment carries a larger version of the same problem than I first thought.

## The three blockers, verified

**1. The right slot.** Correct, and correct for the right reason - the rule is stated at the branch that renders it and `on_event` is pinned to the same predicate in the same order. I walked all thirteen reason codes:

| reason | retryable | details | renders | on_event right |
|---|---|---|---|---|
| `CardLost` | yes | no | Retry / **Exit** | Exit |
| `WipeStopped` | yes | yes | Retry / **Details** | Details |
| `Partial` + `pass_truncated` | yes | yes | Retry / **Details** | Details |
| `Partial` (uncut) | no | if any caveat | Finish / Details | Details |
| `OverCapacity` | no | yes | Finish / Details | Details |
| `WipeUidChanged` | no | if failures or cut | Back / Details | Details |
| the other seven | no | no | left only, no right button | unreachable |

No row disagrees. Retry + Details with Back as the exit was the right shape, and `case WipeStopped: return true` renders a button now, so the note that arm exists to reach has a route. The rule as *written* has a gap - see `write_fail.c:374` - but the code does not.

**2. The cut clone.** Fixed on the route you fixed. Traced a 256-block source cut at block 40 with five real refusals below it: `Cloned 35/256` / `Not written: 221` / `Timed out at block 40`, Details lists the five and only the five. `list_upto` does the work, and counting `has_block_list` over that same range rather than over `failed_count` is the detail that keeps the title honest when the cut lands before anything was refused.

`pass_truncated` over `sweep_truncated` is right, and so is putting `is_retryable` on the result. My `:26` is withdrawn on your terms rather than merely conceded - the parameter is load-bearing now.

**3. The CHANGELOG claims.** Both corrected, and the prefix limit under the second is what makes it true rather than just weaker.

## Your open question - the budget

**Keep the guard, and keep 10s.** You argued it correctly: dropping it trades a fabrication the user cannot check for one they can, and a cut that claims capacity is a verdict about someone's hardware from evidence that stopped mid-run. Under-claiming leaves a correct report and an actionable button. That is the right direction for the error.

I would not take 20s either. It doubles the window where Back is swallowed on every card to buy a diagnosis on one shape of card, and #252 is what users actually hit. A clone-specific budget is the real fix and it is not this PR's.

## Blocking

**`iso15693_poller.c:1131`** - the Fail guard is ahead of the Partial test and reads only the counts, so a cut clone that accepted nothing renders "no data block took", with no cut note, no Details and no Retry - the outcome with the *most* blocks above the cut is the one that denies the button. Full trace in the thread; the fix is one conjunct.

## Claims that do not match the code

Eleven threads. This is the bulk of the round, and I want to be plain about why I am still weighing comments this heavily at round 6: three of them are comments contradicting *other comments in the same delta*, and two are user-facing. That is the failure mode a 43% comment file has, and it is a better argument for the cut than the percentage was.

Self-contradictions this delta created:

- **`iso15693_poller.c:260`** - the field comment on `pass_truncated` still says "Clone mode: unused (stays false)", 365 lines above the clone setting it. The rename updated the macro name inside and left the scoping.
- **`iso15693_poller.h:157` and `:38`** - "the blocks above the cut are recorded as failures" is clone-only. A cut wipe has no back-fill. `write_fail.c:53-54` states the correct rule and is why `has_details(WipeStopped)` is unconditional.
- **`iso15693_poller.h:164` and `:242`** - the "cut above the claim, total far below" card cannot exist. Past the advertised count the sweep terminates after 8 consecutive absences, so that gap is bounded by 8. The genuinely large gap is the *other* branch, which `partial_details.c:78-79` already describes correctly.
- **`nfc_magic_app_i.h:136`** (and `write_fail.c:131`) - illustrates `WipeStopped` with the refuses-every-write card, which has `wiped == 0` and short-circuits to `NothingWiped`. You say so yourself at `write_fail.c:232-233`.

User-facing:

- **`CHANGELOG.md:138`** - the gen3 entry. Every technical claim in it checks out against proxmark master; the sentence about *this app* does not.
- **`CHANGELOG.md:68`** and **`file_select.c:111`** - "matching Gen2 and Classic, which also write without one" is false for Classic. `mf_classic_write_check.c:25-26` sets `uid_locked = true` unconditionally and that scene has no zero-problems shortcut, so Classic always prompts. Gen2 alone is unprompted. My fault as much as yours - I checked the shared routing last round and not the Classic precondition.
- **`CHANGELOG.md:41`** - "always reported as such" is not what the code does, on two routes.

And: `iso15693_poller.c:175` (the cost derivation, worse than I first said), `iso15693_poller.c:1060` (the third "load-bearing" use credits the prefix property for what `pvPortMalloc` does), `write_fail.c:374` (the stated rule omits its third case), `write_fail.c:19` (a `WipeUidChanged` cut gets no Retry), `write_fail.c:226` (`NothingWiped` is the wipe outcome where `uid_verified` is always false and unreachable), `partial_details.c:92` (off-by-one at `cut == advertised`).

Two minors not worth threads: `iso15693_poller.c:750` cites #252 for Back-swallowing, which your own mapping at `scene_write.c:552-553` calls #253; and "blocks_total is the highest block that ANSWERED" (`iso15693_poller.h:163`, `write_fail.c:128`) is `highest_present + 1`, a count rather than an index, and the worked example four lines later uses count semantics.

## Simplify

The one that will amuse you: **`source_uses_gen1_blocks:1632`** still hand-rolls the all-zero loop. `block_is_empty` is 3-of-4, and the missed site is inside a function this delta edited. Your own words last round: *"the same shape as the is-buffer-all-zero helper last round, where you counted three and there were four."*

Also: `has_block_list`'s `+ over_capacity` term is provably dead; the bitmap idiom wants a **pair**, not a trio, because the poller never tests a bit; and **re-scope the render table before you build it** - 12 branches, only 4 have static bodies, and `wipe_stopped` arrived dynamic with a conditional third line, so this delta moved the ratio away from a table. What is actually duplicated is twelve identical `FontPrimary` title calls. `{reason -> title}`, not `{reason, title, body}`.

The compact-UID formatter has four copies (`iso15693_info.c:18`, `write_fail.c:287`, `:305`, `write_confirm.c:39`). The fold relocated one, it did not add one - flagging it for the comment cut, not against this delta.

## Verification

Clean at Unleashed API 88.0, zero warnings, `ufbt format` clean, no tracked artifacts. `dist/nfc_magic.fap` is 149,660 - up 412 from `f8eb8164` on this toolchain against your +256 on yours, which is a baseline difference rather than a disagreement.

Checked by hand so you need not re-argue them:

- dropping `i < advertised` from the tail-drop is exactly equivalent - `advertised` is `iso15693_3_get_block_count(target)` at `:826` and `block_held_data` range-checks the same object
- dropping the bitmap bound from both clone loops is safe - `source_count` is clamped at `:565` and assigned nowhere else
- `write_block_retried` and `block_is_empty` are behaviour-preserving at every site, including the deliberate missing break before the last delay
- the folded confirm scene's `on_event` and `on_exit` are identical to the deleted one's; `text_height` 38 preserved, `uid_str` freed before the view switch exactly as before
- `cut_block` is <= 255 in both modes, so every `(uint8_t)` cast is lossless, and neither `block - absent_run` nor `block + 1 - absent_run` can underflow
- your gen4 citations are exact: `gen4_poller.c:281`/`:358`/`:460` are each `current_block++`, and the callback dispatches only on Ready with no error budget

And I verified your proxmark gen3 facts against master rather than taking them on trust - `hf 15 csetuid --v3` at `cmdhf15.c:3405`, `hf 15 cfinalize` at `:4316`, UID in 0x10/0x11 and the signature `A5 2B 44 2C` / `21 AE 93 00` in 0x14/0x15 at `:3313-3320`. All correct, and a good entry to have added.

