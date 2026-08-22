
## `base_pack/nfc_magic/CHANGELOG.md:41`

<!-- id=3823546168 -->

"A sweep the clock cut short **is always** reported as such" is stronger than the code. Two routes drop it:

- **CardLost.** Both cut paths can hand off with the flag set: the clone at `:726` (the back-fill guarantees `any_failure`, so a card gone at the post-cut check goes to CardLost), the wipe at `:1100`. `has_details(CardLost)` is false, so nothing renders the cut. Benign - the message is accurate and CardLost is retryable - but not "always".
- **NothingCloned**, per the `:1131` thread, where it is not benign.

Fix the guard and this reduces to the CardLost case, at which point I would only soften it to "is reported as such whenever the run reaches its own result screen" - or drop the word.

## `base_pack/nfc_magic/CHANGELOG.md:68`

<!-- id=3823546157 -->

"matching Gen2 and Classic, which also write without one when their pre-write checks find nothing to report" is false for Classic, and `file_select.c:111-112` carries the same claim into the source.

Gen2 has the shortcut: `gen2_write_check.c:24-31`, `if(problems.all_problems == 0) { next_scene(Write); return; }`.

Classic does not. `mf_classic_dict_attack.c:229` routes to `NfcMagicSceneMfClassicWriteCheck`, and that scene has no zero-problems shortcut at all - instead, `mf_classic_write_check.c:25-26`:

```c
// Set the uid_locked problem to true as we have a Mifare Classic card
problems.uid_locked = true;
```

so `problems_count >= 1` unconditionally and the user always presses through at least one WriteProblems screen. Classic is never unprompted.

This one is as much mine as yours - I checked the shared `write_check_common` routing when you raised the parallel and did not check whether Classic can reach zero problems. It cannot.

The argument still stands on Gen2 alone, which is the closer analogue anyway (same poller family, card-derived checks). Minor second point: "card-derived" is inexact even for Gen2 - half of them come from `gen2_poller_check_source_problems(instance->source_dev)`, which is the file, not the card.

## `base_pack/nfc_magic/CHANGELOG.md:138`

<!-- id=3823546151 -->

I verified the whole proxmark claim against master rather than taking it on trust, and every technical part is right: `hf 15 csetuid --v3` at `cmdhf15.c:3405`, `hf 15 cfinalize` at `:4316` ("Finalize a magic V3 tag (irreversible)"), UID in `ISO15_MAGIC_V3_BLK_UID_LO/HI` = 0x10/0x11, signature `A5 2B 44 2C` / `21 AE 93 00` in 0x14/0x15 at `:3315-3320`. Good entry to have added.

What is wrong is the sentence about this app:

> Such a card reports "not a magic tag" here, and a wipe or clone writes over those blocks like any other data.

True of clone and Write UID. **Not true of the wipe**, which has no magic check at all - `nfc_magic_scene_iso15693.c:61` goes menu -> confirm -> write, and `iso15693_poller.c:1182` dispatches straight into `wipe_blocks` with no backdoor probe and no detection step. Any ISO15693 tag presented to Wipe gets swept.

So on an un-finalized gen3 the sweep zeroes 0x10/0x11 - the UID registers - and 0x14/0x15, the signature proxmark reads to decide the tag is still in configuration mode (`cmdhf15.c:3540`: "signature in blocks 0x14/0x15 not found - already finalized or not a V3 tag"). The card comes out with a moved UID and no longer identifiable as re-writable.

Same hazard class as gen1's 56/57/62/63, which this branch takes seriously enough to carry an open-question comment and a post-power-cycle UID re-check. Worth noting that re-check already covers the identity half here for free - a gen3 wipe surfaces `uid_changed` like any other. It is 0x14/0x15 that nothing speaks for.

**I am not asking for gen3 handling.** The entry is right to declare it out of scope. I want the sentence not to imply a protection the wipe does not have, because the reader it is written for is the person about to wipe one. A clause is enough.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:47`

<!-- id=3823546276 -->

`sizeof()` on the array is right only because the element type is `uint8_t`, and it is now spelled three times (`:47`, `:573`, `:1628`) where it used to be one. `COUNT_OF` is in `furi/core/core_defines.h`.

Not a bug today. It is the kind that appears the moment someone widens the array to `uint16_t` for a block index above 255 - which this file already contemplates elsewhere.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:175`

<!-- id=3823546178 -->

Keeping this thread open rather than resolving it. The rename and the rule are in and both are right; the arithmetic under them is worse than I said last round.

**"a 4-byte zero write"** - the wipe's payload is `iso15693_3_get_block_size(target)` clamped to 32 (`:827`, `:845`). It is 4 on your sample card, not in the code.

**"up to 32 bytes, so up to 8x the frame"** - 8x is the payload ratio, not the frame ratio. The frame also carries flags, command, block number and CRC, so 4 -> 32 bytes of payload is roughly 2.5-4x the frame. And by this comment's own accounting at `:164-166`, ~15ms of the 40-70ms per refused block is `furi_delay_ms`, which no payload size touches at all.

**"and since the probe went onto every persistent failure rather than only the empty ones, more again"** - this is the one that does not survive. The wipe already reads on *every* refused block, unconditionally, at `:930-932`. After your change the two passes have the same per-refused-block shape: 3 writes, 3 waits, 1 read. The clause is true of the clone against its own earlier revision, but the sentence is comparing it against the wipe.

Which inverts the conclusion, or at least removes its support. On the same geometry the clone is not the more expensive pass - the wipe additionally pays a card-present inventory every 8 absences (`:966`) and a full-run re-probe (`:1000-1020`). The clone is dearer only when the source's block size exceeds the target's, and then by the frame ratio rather than the payload ratio.

The *rule* is still right and worth keeping: the value has to be defensible for the more expensive of the two. It is the identification of which one that is, and by how much, that wants narrowing to what the code guarantees.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:260`

<!-- id=3823546093 -->

Self-contradiction created by the rename. "Clone mode: unused (stays false)" - the clone sets it 365 lines below at `:625-626`, which is the headline fix of this delta.

The comment came over from `wipe_truncated` with only the macro name inside it updated. The very next field's doc already reasons about both modes, and `iso15693_poller.h:156` says "BOTH modes" in as many words.

Worth fixing rather than shrugging at, because this is the field a reader consults to decide whether `pass_truncated` needs mode-gating at a call site - and `nfc_magic_scene_write.c:426-428` does mode-gate on it. Someone who believes this comment concludes that gate is dead and deletes it, which puts a cut clone on the wipe-specific "Wipe stopped" screen. That is this round's blocker running backwards.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:650`

<!-- id=3823546259 -->

The bitmap idiom, since you have it on the deferred list and this round added a site. Six in this file - set at `:650`, `:699`, `:940`, `:948`, clear at `:1017`, `:1076` - plus a new **test** in `partial_details.c:33`.

Refinement worth having before you do it: the poller never tests a bit, so this wants a **pair**, not a set/clear/test trio. The test site lives in another translation unit and is better served by the `any_index` helper on that thread, which deletes it.

The payoff is not the line count. `:1017` and `:1076` are `&= ~` sitting inside two of the densest reasoning blocks in the file, three and five lines from a `|=`. Named calls make a slip visible at a glance, which is the whole reason this is on the list.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:1060`

<!-- id=3823546188 -->

The third of the three uses credits the wrong fact, and it is my two findings being fused rather than yours.

"it is what makes the zeroed-entry argument at the read-back note deterministic" - the prefix property does not do that. `pvPortMalloc`'s `memset` does, which is what the read-back note itself says at `:925-929`: "an unread entry is genuinely zeros rather than heap residue - which is what makes block_held_data deterministic."

Two independent disassembly facts: one says the cache is a prefix (`read_blocks` returns at the first failure), the other says unread entries are zeros (`pvPortMalloc` zeroes). The prefix property tells you a dropped block *was read*; the memset tells you an unread entry *reads as empty*. Neither implies the other, and the keep branch leans on both.

The other two uses hold - the divergence bound is sound and the "Not closed" attribution is defensible - so this is "three times over" wanting to be two, plus a pointer to the memset for the third. Worth getting exact in a paragraph that closes "It is worth not re-deriving."

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:1131`

<!-- id=3823546078 -->

**Blocking.** This guard runs ahead of the Partial test at `:1141` and reads only the counts. `pass_truncated` is never consulted, so a clone the clock cut can land here.

Trace - gen2 clone, ~200+ block source, target takes the backdoor UID and refuses every data block:

1. `:619` the clock trips at block C; `pass_truncated = true`, `pass_cut_block = C`, `break`.
2. `:695-701` the back-fill records every block in `[C, source_count)` as a failure. Correct, and your design.
3. `:726` card still present, so no CardLost.
4. `:756-760` `failures_are_top_tail` is false, so `over_capacity` folds in. Now `failed_count == blocks_total`.
5. **this line** -> `Fail`, not Partial.
6. `scene_write.c:487-490` -> `NothingCloned`.
7. `write_fail.c:249-262` renders **"Clone failed / The UID was written, but no data block took. The card has UID only."**
8. `has_details(NothingCloned)` falls to `default: return false` - no Details, so the cut is stated nowhere.
9. `is_retryable` does not cover it - no Retry.

The card refused the blocks below C. Blocks `[C, N)` were never transmitted. That is the sentence `8841a28c` exists to delete, still standing one branch earlier in the same function - and it is the outcome with the *most* blocks above the cut, which by your own `write_fail.c:22-28` argument has the strongest claim on Retry.

`iso15693_poller.h:38-40` states the opposite of what happens: "either mode cut short by the wall-clock bound ... the run's own job is left undone, **whatever the counts say**." The counts are exactly what decides it here.

Minimal fix, and I think it is the right one rather than the cheap one:

```c
if(clone && !instance->pass_truncated && instance->clone_blocks_total > 0 &&
   instance->clone_failed_count >= instance->clone_blocks_total) {
```

It then falls to Partial and everything you built this round handles it: `Clone partial` / `Cloned 0/N` / `Not written: N` / `Timed out at block C`, Retry + Details, and Details lists only the blocks genuinely refused below the cut. The guard's stated purpose - no Finish button under "Cloned 0/28" - survives untouched, because a cut run gets Retry and names the clock instead.

Reachability is narrow: zero accepted blocks, plus a source big enough to spend 10s on refused blocks, so roughly 150-250 by your own 40-70ms figure and fewer with 32-byte blocks. Narrow, but it is the card the `nothing_cloned` screen was written for in the first place.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:1632`

<!-- id=3823546245 -->

`block_is_empty` is 3-of-4. This loop is exactly it, the helper is at `:522` and in scope, and `block_size` is already `uint8_t`:

```c
if(!iso15693_poller_block_is_empty(data, block_size)) return true;
```

Flagging it because the miss is inside a function this delta edited - the hunk right above swapped its local `gen1_blocks[]` for the shared array and left the inner loop hand-rolled. Your own line last round: *"the same shape as the is-buffer-all-zero helper last round, where you counted three and there were four."*

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:38`

<!-- id=3823546110 -->

Same clone-only claim as the `pass_truncated` field doc: "since the unreached blocks are recorded as failures". True of the clone, not of a cut wipe.

Separate thread because this is the entry a *caller* reads to decide what Partial means, and the two together make the back-fill look like a property of truncation rather than of one pass. Fixing one and not the other leaves the contradiction intact.

Also the clause my `:1131` thread quotes against the code - "whatever the counts say" - lives here. Once the Fail guard reads `pass_truncated` this sentence becomes true; until then it is the clearest statement of the bug.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:157`

<!-- id=3823546100 -->

"The blocks above the cut are recorded as failures" is a clone-only mechanism, stated under a heading that scopes it to both modes.

A cut wipe records nothing above the cut. `wipe_blocks` breaks at `:893` and has no back-fill - the clone's is `:695-701`, with no wipe counterpart. Both sites that set a bit in the sweep (`:940`, `:948`) are inside the loop body the deadline check gates, so every set bit sits below `pass_cut_block`. And `blocks_total = highest_present + 1` is itself at or below the cut, so the "derived by subtraction" hazard this sentence invokes does not exist for a wipe: the unreached blocks are outside the denominator, not inside the numerator.

Your own code states the correct rule at `write_fail.c:53-54` - "the blocks above the cut were never attempted, so they carry no bitmap bits" - which is precisely why `has_details(WipeStopped)` is unconditional instead of `failed_count > 0`. The header and the scene disagree; the scene is right.

The consequence to watch is `write_fail.c:146`, which derives the wipe's "Cleared" figure as `blocks_total - failed_count`. Under this header's rule that subtraction looks wrong on a cut wipe; under the actual behaviour it is fine. Someone auditing that line from the header chases a bug that is not there, or "fixes" it into one.

Keep the BOTH-modes sentence, attribute the back-fill to the clone.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:164`

<!-- id=3823546119 -->

This gap cannot exist on the card named, and the card that does produce a large gap is in the other branch.

"on the card this bound was designed for - one that refuses every write and still serves a read at every address - the sweep walks well PAST the advertised count **while proving nothing present**, so the cut can exceed the advertised count while the total sits **far below** it."

That card proves *every* block present. A refused write falls through to the read at `:930-932`; a successful read calls `wipe_note_present`, which sets `highest_present = block`. So `blocks_total == cut_block` exactly.

More generally the two conditions are incompatible. To get past `advertised` at all the sweep must never accumulate `ISO15693_POLLER_WIPE_ABSENT_RUN` (8) consecutive absences, because past the claim that run ends it. So whenever `cut_block > blocks_advertised`, the gap is bounded by 8. "Far below" is unreachable in that regime.

The real large-gap case is the opposite one, and `partial_details.c:78-79` already has it right: a card claiming 200 while holding 10 gives `blocks_total = 10` with the cut somewhere around 150-180 - and there the cut is *below* the advertised count. Below the claim the sweep never stops on absence alone, which is exactly what lets the gap grow.

So the field is right that the two figures are not interchangeable, and right that a cut can exceed the claim. It has fused two different cards into one that exhibits neither. `:242` carries the same fusion in the `start_wipe` header.

## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:242`

<!-- id=3823546131 -->

Same fusion as the `cut_block` field doc above: on the read-everywhere card `blocks_total == cut_block`, so it does not "sit below" the advertised count while the cut sits above it - both are above.

Separately, and cosmetically: this line is 111 columns against the file's ~103 norm. You appended to the sentence and it did not get rewrapped. `.clang-format` has `ColumnLimit: 99` with `ReflowComments: false`, so `ufbt format` will never catch it - which is also why `iso15693_poller.c:166` has sat at 155 columns since `78e1d168` without either of us noticing. That one is mine to have missed. Both are cosmetic, and the comment cut is the pass that touches these lines anyway.

## `base_pack/nfc_magic/nfc_magic_app_i.h:136`

<!-- id=3823546143 -->

The archetype is right about the mechanism and wrong about the screen. A card that "refuses every write, still serves every read" has `wiped == 0`, so `iso15693_poller.c:1201` short-circuits to `Fail` before `VerifyWipe` and it lands on **"Wipe failed"**, never on WipeStopped.

You say exactly this 100 lines away, at `write_fail.c:232-233`: "it is also the one that ends here: nothing accepted, so the wipe short-circuits to this screen before any of the truncation reporting." Two comments in one delta disagreeing about where that card ends up.

The claim that matters - the cut index can sit above the advertised count - survives with a one-block change to the example: a card that accepts *some* writes and then answers reads everywhere. `write_fail.c:130-133` carries the same archetype and needs the same edit.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_file_select.c:111`

<!-- id=3823546285 -->

Same claim as `CHANGELOG.md:68`, and same correction: Classic never reaches the write unprompted.

`mf_classic_write_check.c:25-26` sets `problems.uid_locked = true` unconditionally and that scene has no zero-problems shortcut, so `problems_count >= 1` always and at least one WriteProblems screen is shown. Only Gen2 has the shortcut, at `gen2_write_check.c:24-31`.

Dropping Classic from the sentence leaves the argument intact - Gen2 is the closer analogue anyway. Worth keeping the rest of this comment as it stands; the ISO15693 rationale and the Gen1/Gen4/USCUID-UL clause both check out.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_partial_details.c:31`

<!-- id=3823546254 -->

Two small things.

`listed` has exactly one use, `> 0` on the next line, so this loop is an existence test written as a tally, early-exiting nowhere, and it is the only raw bit arithmetic in the scene layer. A `nfc_magic_partial_details_any_index(bitmap, count)` next to `append_indices` would make "counted over the same range that will be printed" structurally true rather than a comment.

And the `+ over_capacity` term is dead. `over_capacity` survives non-zero only down the `failures_are_top_tail` branch (`:752-753`), which requires `!pass_truncated`, which forces `list_upto` to the full range - and the bit is set at `:650` unconditionally, before the bucketing at `:683-687`. So `over_capacity > 0` implies `listed >= over_capacity > 0`. The sum is `2 x over_capacity` in that state and `listed` everywhere else, and `> 0` is identical either way. Harmless, but it reads as though the terms are complementary when one strictly contains the other.

Related: `:17-22` and `:29-30` both describe the clone's back-fill unconditionally, while the code they annotate runs in both modes. On a cut wipe the bound is a no-op and neither sentence is true - same clone-only-stated-as-both-modes shape as `iso15693_poller.h:157`.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_partial_details.c:92`

<!-- id=3823546236 -->

Off-by-one at the boundary. The branch above is `cut_block < blocks_advertised`, so `cut_block == blocks_advertised` falls here and prints "at block N, **past** the N this card claims" - when the sweep stopped exactly at the claim, having attempted precisely the claimed range and nothing more.

`<=` on the first branch, or reword this one to "at the N this card claims". The first branch's sentence ("Blocks above that were never attempted and may still hold data") is the accurate one at equality, since block N itself was not attempted.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:19`

<!-- id=3823546224 -->

`Partial && pass_truncated` gets Retry and `WipeStopped` gets Retry, but `WipeUidChanged && pass_truncated` does not. `scene_write.c:425` takes the UID branch first, so a wipe that was both cut and moved the UID lands on Finish - while `has_details:56` already grants that same case the truncation note.

Same "data may sit above the cut" condition, opposite answer, and what decides it is branch ordering rather than an argument.

I can see a real case for it staying: re-running a wipe against a card whose identity has already moved is not obviously helpful, and on gen1 it is another pass over 56/57. If that is the reasoning it belongs at the ordering in `scene_write.c`, where a reader will look for it. If it is not, this predicate wants a third clause.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:226`

<!-- id=3823546230 -->

`a2a905ca` says it puts the "check never ran" fact on every wipe outcome, and its message says the wipe has three outcome screens. `partial_details.c:121-123` repeats the three. There are six reachable wipe reason codes, and this is the one that still has no route:

| screen | `uid_verified` false? | reachable |
|---|---|---|
| WipeComplete | yes | inline third line |
| WipeStopped | yes | `has_details` always true |
| Partial (wipe) | yes | Details via `failed_count > 0` |
| WipeUidChanged | no - observing a change requires an answer | n/a |
| CardLost | yes | no Details, but the card left, so it is moot |
| **NothingWiped** | **always** | **neither** |

`:1197-1204` skips the power-cycle and `VerifyWipe` entirely on `wiped == 0`, so `uid_verified` is false by construction here, and `has_details` gives this reason no Details button.

The `wiped == 0` short-circuit is pre-existing and I am not asking you to change it in this PR - only the coverage claim is new. But its reasoning ("no write landed that could have moved the UID") is the one inference this file refuses to draw anywhere else: the sweep did send three WRITE BLOCKs each at index 56 and 57 before the cut, and `write_identity`'s own comment at `:428-430` says a tag can apply a write without answering. So on an armed gen1 card this is a wipe that can move the UID, report "Wipe failed", never run the check, and never say the check did not run.

Gen1, untestable without a card, so the same bucket as everything else gen1 - file it rather than fix it. Worth correcting the count either way.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:249`

<!-- id=3823546264 -->

Re-scoping the render table you still owe me, before you spend a round building the wrong one.

I counted the branches: **12**, and only 4 have static bodies - `nothing_cloned`, `gen1_failed`, `uid_unverifiable`, `empty_source`. Seven build theirs dynamically, several with conditional extra lines, and the twelfth is the multi-reason `else`. `wipe_stopped` was the newest arrival and it is dynamic with a conditional third line, so this delta moved the ratio *away* from a table. A table covering 4 of 12 splits the screen across two mechanisms and makes "what does reason X render?" a two-place lookup - worse than one chain.

What is actually duplicated is **twelve identical** `widget_add_string_element(widget, 64, 0, AlignCenter, AlignTop, FontPrimary, <title>)` calls, plus twelve `widget_add_string_multiline_element` varying only in `(x, y)`.

So: `{reason -> title}` plus one hoisted call, not `{reason, title, body}`. That isolates a real invariant - every screen has exactly one centred FontPrimary title - and it is the version worth doing. The fuller form is not the right target and will not become one.

## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:374`

<!-- id=3823546205 -->

The rule is right about the case that broke and silent about a third case that exists.

"the RIGHT slot means Details whenever `has_details` is true, and Exit only when it is false."

In the non-retryable branch at `:404-421`, `has_details == false` renders **no right button at all** - not "Exit". Exit only appears in the retryable branch. The full rule is three-way: Details if `has_details`; else Exit if retryable; else nothing.

Flagging it because of the instruction this comment closes with - "Add a reason to either predicate and re-read this." Someone adding a reason and reasoning from the two-way version concludes a non-retryable reason without details gets an Exit button, which is not what happens. The table I ran is in the summary; the code is right, only the statement of it is short a branch.
