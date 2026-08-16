
## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:14`

<!-- id=3792177313 -->

This comment is on the wrong function. Paragraphs 1-2 answer "does this outcome have anything behind
Details" and describe the switch below - including "UID-changed is here because it PRE-EMPTS the
partial reason code", which is a statement about a case that appears in `has_details` and is
deliberately *absent* from `is_retryable`. Only the third paragraph belongs here.

`has_details` - the one with the non-obvious switch - has no header at all. Same slip as the wipe
header at `iso15693_poller.c:700`: a helper inserted directly beneath an existing doc block.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:26`

<!-- id=3792177311 -->

`instance` is unused. `has_details` needs it because it reads `iso15693_result`; this one is purely
over `reason`, and symmetry isn't worth an `UNUSED` in a two-line predicate. Dropping it also makes
the asymmetry between the two visible at the call sites, which is a hint worth leaving.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:40`

<!-- id=3792177308 -->

This arm is unreachable as written - `is_retryable(WipeStopped)` is true, so `on_enter:341` takes the
other branch and the `Details` button below is never added. The comment is right about why the note
needs a route; it just doesn't get one. See the thread on `:341`.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:71`

<!-- id=3792177320 -->

Nothing to fix - recording that I checked the tone change. `wipe_complete` can no longer coexist with
`sweep_truncated` (`success_or_partial:1068` forces Partial), so dropping `wipe_cut_short` is sound
and the tone now agrees with the event rather than contradicting it. Which was the point.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:100`

<!-- id=3792177314 -->

`uid_verified` is read here and nowhere else. The wipe has three outcome screens and this is one of
them: a card that never comes back from the `NfcCommandReset` exhausts
`ISO15693_POLLER_WIPE_VERIFY_ACTIVATIONS` and reports `success_or_partial` with `uid_verified` still
false, so if the run is Partial for any other reason the user lands on "Wipe partial" or "Wipe
stopped" and neither mentions the check.

This is the shape of the `sweep_truncated` item from last round, recurring with the new field - and
the field's own doc at `iso15693_poller.h:151-154` says "a caller reporting success should say so",
which two of three callers don't.

It bites hardest on the card the check exists for. The sweep zeroes blocks 56/57 at *index* 56/57,
long before any plausible clock cut, so a gen1 card armed by an earlier UID write has already had its
UID registers written by the time a truncation is decided - and "Wipe stopped" then offers Retry
without saying the identity check never ran.

The Details screen is probably the right home for it, the way you handled the truncation note.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:117`

<!-- id=3792177318 -->

`blocks_total` is `highest_present + 1` - the highest block that *answered* - not where the sweep
stopped. The cut index is the local `block`, which never leaves the function.

Card advertising 64, blocks 0-49 clean, 50-54 answering neither, clock fires at the top of iteration
55: `block = 55`, the tail-drop drops 50-54 (cache holds nothing for them), `blocks_total = 50`.
Screen reads "Stopped at 50 of 64" for a sweep that attempted 55.

There is a second reading problem on the card this bound was actually designed for - one that answers
reads at every address and never accumulates a run. That sweep walks well past the advertised count
before the clock fires, so this renders **"Stopped at 200 of 64"**, which parses as falling short of
64 when it exceeded it. Carrying the cut index into the result as its own field fixes both, and
`iso15693_poller.h:148` and `nfc_magic_app_i.h:134` with them.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_write_fail.c:341`

<!-- id=3792177306 -->

**Blocking.** This renders `Retry` / `Exit` for `WipeStopped`, and `on_event:396` sends the right
button to `has_details` - which is `true` for `WipeStopped` - so the button labelled **Exit opens the
Details scroll view**.

Before this delta the two sets were disjoint: `is_retryable` was `{CardLost}` and
`has_details(CardLost)` falls through to `default: return false`, so `on_event`'s `else` branch
happened to mean "the retryable outcomes' Exit". Putting `WipeStopped` in both broke the premise
that `else` silently relied on, and nothing in either function states it.

Two consequences:

1. A control labelled Exit navigates deeper. Back still escapes, so it is not a trap - but pressing
   it replays the error tone on return.
2. The `case WipeStopped: return true` arm below never renders a button, so the truncation note it
   exists to reach has no labelled route on its own screen.

Note the layout is the real constraint: this reason wants Retry, Details *and* Exit, and the branch
has two slots. Since `on_event:407` already exits on Back, the cheapest honest shape is **Retry +
Details** for `WipeStopped`, leaving Back as the exit - which is what the "Exit" button on the
card-lost screen duplicates anyway. If you'd rather keep Exit, `on_event` has to test
`!is_retryable(...) && has_details(...)` and the arm below becomes dead and should go.

Whichever you pick, the two functions need to agree about which slot means what, and the reason they
have to should be written down - that is the part `b8687ed6` fixed one layer down and left open
here.


## `base_pack/nfc_magic/CHANGELOG.md:42`

<!-- id=3792177367 -->

**Blocking.** "Where the difference *is* a fault - a dead stretch inside the claimed range ... - it is
reported as such rather than hidden in the counts."

The most likely shape of that fault is the one case this cannot report. Card advertising 64, blocks
50-63 already silent when it was presented: activation reads 0-49 and times out at 50 (activation
still succeeds - the filter maps Timeout to None), the floor rule sweeps 50-63, the run trips, the
re-probe confirms all 14 absent, and the tail-drop finds `block_held_data` false for every one
because the cache is a prefix that stopped at 50. `failed_count == 0`, `blocks_total == 50`,
**Success** - "Wipe complete. Cleared 50 blocks. Card claims 64.", success tone, Finish, no Details.
Which the bullet immediately above tells the reader is benign.

Only an *interior* dead stretch - something answering above it - is reported.

`iso15693_poller.c:985-986` says this plainly ("Not closed"). I don't think the code gap is closable:
the prefix property means a fake-flash card and a dead-at-presentation card produce byte-identical
observations, and you already chose the only disposition that doesn't re-break the 70/64 card. So
this is a documentation fix, not a code one - bring the claim down to what `:44-48` already does
correctly.

Worth considering separately, and your call: `blocks_total < advertised` and `blocks_total >
advertised` are not the same situation. The second is unambiguously benign (programmed count, cloned
from a smaller source). The first is fake flash **or** dead memory, and the app cannot tell which -
yet it resolves that ambiguity toward reassurance, with the chime and the word "complete". The
"Wipe complete" screen's third line is free whenever `uid_verified` is true.


## `base_pack/nfc_magic/CHANGELOG.md:47`

<!-- id=3792177369 -->

Second, separate error in the same bullet. "a block whose contents **were read** when the card was
first activated provably exists and provably held data".

The code tests **non-zero content**, not "was read". `iso15693_poller_block_held_data` returns true
only on a non-zero byte, so a block that was read at activation, provably exists, and returned all
zeros is still dropped. Your own helper doc gets this right - "Only TRUE means anything ... FALSE
proves nothing" - and this sentence loses exactly that distinction.

"Below the card's own claimed count the wipe distinguishes them" also needs a clause: it doesn't,
above the first block that failed its activation read, because the cache stops there. That is the
same prefix fact behind the `:42` thread.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:58`

<!-- id=3792177344 -->

Confirmed against `firmware.elf`: `furi_delay_ms(100)` at `0x080445c6`, 40 x 100ms. The corrected
figure is right and the one it replaced was not.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:155`

<!-- id=3792177338 -->

The derivation behind this number is a wipe's **4-byte zero write**. `8a65838a` reuses the same
constant for the clone's data pass, whose payload is the source's block size - up to 8x per frame,
plus the source read. So the clone sits materially closer to the cut than "256 blocks all accepting
lands around 3-4s" implies, which makes the truncated-clone path in the `:557` thread more reachable
than this analysis suggests.

Either give the clone its own constant or note here that it borrows this one and why that is still
safe.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:557`

<!-- id=3792177322 -->

**Blocking.** `pass_truncated` never leaves this function - its only consumer is
`failures_are_top_tail` at `:687`. Nothing reaches the result struct; `sweep_truncated` is wipe-only.

So a clone cut at block 10 of 256 with the card still present renders "Clone partial / Cloned 10/256
blocks / Not written: 246", Details titles the list **"Blocks not written"** and enumerates all 246
indices, the truncation note at `partial_details.c:57` is gated on `sweep_truncated` so it does not
print, and the button is **Finish** - not Retry, though re-running is exactly what would write them.

The user is told their card refused 246 named blocks. Nothing was sent to any of them.

This is your own argument from `f8eb8164`, applied to the report instead of the classifier: *"those
blocks refused nothing, they were never attempted."* You used it to stop the app fabricating "Card
too small"; it equally stops the app fabricating a list of blocks the card wouldn't take.

The wipe already has the machinery - promote `pass_truncated` to an instance flag, let
`sweep_truncated` mean "this pass was cut" for both modes, and the `partial_details.c:57` note and
`is_retryable` cover the clone for free. The field doc's "False for a clone" then goes.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:618`

<!-- id=3792177337 -->

This paragraph and `:603-605` both still say the probe is paid only for empty failures, directly above
the paragraph that reverses them ("Run the probe on EVERY persistent failure, not just the empty
ones"). The new rationale was appended rather than substituted.

Keep `:610-616` - the read-answers-therefore-exists argument is still true and still load-bearing.
`:603-605` and `:618-619` should go; the new paragraph already carries the framing.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:649`

<!-- id=3792177324 -->

The back-fill is right to record these rather than let them count as written - the subtraction would
claim all 256 landed. The problem is downstream: nothing distinguishes them from blocks the card
refused. See the thread on `:557`.

Separately, `block < ISO15693_POLLER_BLOCK_BITMAP_SIZE * 8` cannot fire in either loop header -
`source_count` is clamped to exactly that at `:502-504` and never reassigned. It was already
redundant at `:559`; this loop copied it. And the four-way backdoor test is now written out twice
verbatim here and at `:565`, with the same four constants enumerated a third time at `:509-514`. One
`iso15693_poller_is_backdoor_block(block)` predicate covers all three.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:659`

<!-- id=3792177326 -->

`// land on 100%` is no longer true - after a truncation `done` stops at the cut, so the popup's last
frame is wherever the clock landed. Either drop the comment or pass `total` deliberately and say
why.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:687`

<!-- id=3792177327 -->

This guard is right, and it lands on the card it was least meant for.

Its stated target is "a slow-but-healthy card", but this pass's cost is dominated by *failing*
blocks: 3 writes + 3x`VERIFY_RETRY_MS` + - since `7b700064` - a read probe on **every** persistent
failure rather than only the empty ones. At the file's own 40-70ms per refused block, 10s is roughly
143-250 failures.

A card that is genuinely too small presents precisely a long contiguous run of failing blocks. So a
source ~150+ blocks larger than the target truncates and then reports the generic partial instead of
the "Card too small" line this whole apparatus exists to earn. `7b700064` raised that per-block cost
in the same delta that added the guard.

Worth either a longer budget for the clone specifically, or a comment acknowledging the overlap so
the next reader doesn't mistake the suppression for a bug. Not a correctness problem - erring toward
the weaker claim is the right direction - but the interaction is worth stating.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:700`

<!-- id=3792177334 -->

This 26-line header documents `iso15693_poller_wipe_blocks`, but the two new helpers were inserted
between it and that function with no blank line at 725/726 - so it is one contiguous block and it now
sits on `iso15693_poller_block_held_data`.

A reader arrives at a two-line `bool` helper under "Wipe mode: write zeros to every data block the
card PHYSICALLY holds", the read-back classification table, the gen1 56/57/62/63 argument and
"Returns the number of blocks that actually accepted the zero-write". `wipe_blocks` at `:755` is now
undocumented.

Same slip as `nfc_magic_scene_iso15693_write_fail.c:14`. Moving both helpers above `:700` is the
smaller edit.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:855`

<!-- id=3792177342 -->

Recording a verification rather than asking for a change, since the keep branch at `:993` now depends
on this paragraph being exactly right.

Both halves hold, read out of `firmware.elf`:

- `iso15693_3_poller_read_blocks` (`0x08044738`) is a loop of single READ BLOCKs that returns on the
  first error, so the cache is a prefix - it stops at the first block that did not answer.
- the filter is inlined into `iso15693_3_poller_activate` at `0x080448b0` as `cmp #6` / `cmp #9`,
  and in the current header those are `Iso15693_3ErrorTimeout` and `Iso15693_3ErrorNotSupported`.
  Exactly the two you name.

And the "zeroed allocation value" clause is not an assumption: `simple_array_init` does a bare
`malloc` with no per-element init for a byte array, but `pvPortMalloc` tail-calls
`memset(p, 0, size)`. Flipper's allocator zeroes, so those entries are zeros rather than heap
residue - which is what makes `block_held_data` deterministic on a fake-flash card. Your bench pass
on the 70/64 card could not have distinguished "the fix works" from "this heap happened to be
clean"; it does work, for a reason worth one clause here.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c:993`

<!-- id=3792177329 -->

This keep branch and `wipe_note_present` now state opposite rules about the same blocks.

`note_present` says: a block proved present, therefore every absence still open **below** it is
interior memory that answered nothing, so those count. This loop decides block by block, ascending,
and drops each unproven one - including ones below a proven-present member of the same run. Run
56..63 where only block 60 reads non-zero in the cache: 56-59 are dropped, 60 is kept,
`blocks_total = 61`.

**What makes that harmless is a fact worth writing into this comment, because without it this reads
exactly like the round-4 bug.** `iso15693_3_poller_read_blocks` (`0x08044738`) returns at the first
failing block, so the activation cache is a **prefix**: a block proven present implies every block
below it was also read. Therefore a block dropped *below* a proven one always read back as zeros at
activation - it held nothing, and dropping it conceals nothing. The divergence is bounded by the
cache's shape, not by the rule.

That same prefix property is why finding 3 in the summary cannot be fixed in code, and why your
"Not closed" note at `:985-986` is the correct disposition rather than a placeholder. It is the load-
bearing fact of this whole mechanism and it is currently stated only as "at its zeroed allocation
value" at `:859` - which, for what it's worth, I confirmed: `pvPortMalloc` tail-calls
`memset(p, 0, size)`, so those entries are genuinely zeros and not heap residue.

If you'd rather have the rules agree, decide the boundary first - the run's highest cache-proven
member - then clear above it and hand the remainder to `wipe_note_present` once. That also removes
`i < advertised`, which duplicates the bound check `block_held_data` already does against the same
object.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:24`

<!-- id=3792177345 -->

"the sweep ran to the card's top and every block it reached is clear" overreaches in two directions
now.

The sweep also exits at the 256-block ceiling, which sets no flag and yields Success if nothing
failed - and by this file's own estimate 256 all-accepting blocks land inside the 10s budget, so that
exit is reachable. And blocks dropped below the advertised count *were* reached and are not known to
be clear; the parenthetical only excuses blocks "past that top", which reads as the advertised top
rather than the measured one.

Same overreach echoed at `nfc_magic_app_i.h:129-130` ("no block refused") - a block that refuses
every write and reads back zero is deliberately not counted (`iso15693_poller.c:876-880`), so "no
block refused" is false there by design.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:30`

<!-- id=3792177347 -->

The contract change didn't reach this entry: `success_or_partial:1067-1068` now also returns Partial
on `wipe_truncated`, which is the whole point of `9c404bc4`. This is the enum entry a caller reads to
learn what Partial covers.


## `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.h:148`

<!-- id=3792177349 -->

"blocks_total is where it was cut" - it is `highest_present + 1`, the highest block that *answered*,
which is at or below the cut and can be well below it. See the `write_fail.c:117` thread; the two
user-visible strings inherit this.


## `base_pack/nfc_magic/nfc_magic_app_i.h:134`

<!-- id=3792177353 -->

"with blocks the card claims still unattempted" is not what `wipe_truncated` means. The check at
`iso15693_poller.c:819-823` is a position-independent wall-clock test at the top of every iteration -
nothing ties the cut to `advertised`.

The card this bound was designed for is the one from `ISO15693_POLLER_WIPE_MAX_MS`'s own comment: it
refuses the write and still serves a read at every address, so it walks *past* the advertised count
and is cut with every claimed block attempted. Same claim at `iso15693_poller.h:231-232` ("short of
the card's claim"), and it is load-bearing for the "Stopped at 200 of 64" string.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_iso15693_partial_details.c:65`

<!-- id=3792177364 -->

Two problems, both from `blocks_total` standing in for the cut index.

"Blocks above that were never attempted" is false for the trailing run the tail-drop just discarded -
those blocks got three writes and a read each. On the trace in the `write_fail.c:117` thread the
sentence excludes blocks 50-54, which were attempted.

And on a card claiming ~200 blocks that physically holds 10, the sweep spends the whole budget in the
floor rule and is cut around block 180 with `blocks_total == 10`, so this reads "hit its time limit
at block 10" about 170 blocks that were attempted and answered nothing.

It errs conservative in both cases - it over-warns rather than under-warns - which is the right
direction to be wrong in. But it is a factual claim about which blocks were tried, on the screen
whose job is to say where the sweep stopped.


## `base_pack/nfc_magic/scenes/nfc_magic_scene_write.c:539`

<!-- id=3792177356 -->

`05301775` exists to correct this comment's supporting claims, so this is worth catching: gen4 is
enumerated at the top ("the other four magic protocols") and then disappears - accounted for neither
in the safe pair nor the "unsafe **pair**", which the wording asserts is exhaustive.

By this comment's own criterion gen4 belongs in the unsafe set: `gen4_poller_start` uses
`nfc_poller_start` (`gen4_poller.c:813`), `gen4_poller_callback` acts only on
`Iso14443_3aPollerEventTypeReady` and returns `NfcCommandContinue` for everything else
(`gen4_poller.c:799-801`), it has no activation-error budget, and it advances one block per Ready
(`current_block++` at `:281`/`:358`/`:460`). Card gone, Error events discarded, state machine never
called again - the same trap you describe for gen2. The comment this replaced did name it.

Everything else in the rewrite checks out: the gen1a and USCUID-UL-backdoor `nfc_start` claim
(`gen1a_poller.c:370`, `uscuid_ul_poller.c:498`) and the USCUID-direct `NfcCommandReset` claim
(`uscuid_ul_poller.c:376-381`).


## `base_pack/nfc_magic/scenes/nfc_magic_scene_write.c:546`

<!-- id=3792177359 -->

"the linked issue" - name it. You filed #252 and #253 and this paragraph spans both (the 88-second
measurement is #252, needing a reboot is #253). A bare "the linked issue" is not navigable from the
source tree, which is where this comment will be read.

