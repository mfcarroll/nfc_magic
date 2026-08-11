# NFC Magic: a card removed mid-write is not reported on Gen2/Classic or USCUID-UL — every remaining block is retried instead

**Pre-existing.** Found while scoping a change in #250; filing separately because it constrains anything
that touches the shared write scene.

## Observed on hardware

Same procedure each time: start a clone, lift the card the moment the popup switches to "Writing", then
wait.

| protocol | card | on a failed block | result |
|---|---|---|---|
| **Gen2 / Classic** | CUID magic Classic 1K | mark failed, `current_block++`, carry on | **no report.** Popup sits on "Writing / Don't move..." |
| **USCUID-UL** | magic NTAG216 | mark failed, `write_index++`, carry on | **no report** after >1 min, frozen at `Writing 147/231` |
| Gen4 | GTU "Ultimate Magic Card" | `state = Fail`, stop | reports immediately — "Something went wrong while writing" |
| Gen1A | magic Classic 1K | `state = Fail`, stop | reports promptly — same screen |
| ISO15693 | gen2 magic | n/a — checks the card is still present | reports |

The correlation is exact across all five: a poller that **abandons the write on the first failed block**
reports straight away; a poller that **carries on through every remaining block** never gets there in any
tolerable time. Gen4 is the decisive case — it shares Gen2's callback shape and its `Ready`-only event
handling, and it is fine. So the callback structure is not the cause; what the write handler does with a
failure is.

Gen2 has up to 64 blocks left to retry, each paying a failed authentication and a halt. USCUID-UL had 84
pages left, and on the direct engine each failure additionally returns `NfcCommandReset` and triggers a
doomed re-activation before the next attempt.

Which strongly suggests these writes are not *stalled* but *grinding* — the user-visible effect is the
same, but it means the fix is to notice the card has gone, not to add a blanket timeout.

## Consequence

The write popup does not resolve in any usable time. No "Card removed", no partial result, and no report
of the blocks that *were* written before the card left — which for a half-completed clone is exactly what
the user needs.

On Gen2/Classic the escape is worse than merely absent: pressing Back lands in an inescapable loop
between the write-check and write scenes. That is a separate defect, filed alongside this one.

Note also that Back never aborts a write in any case. The scene's `on_exit` calls
`<proto>_poller_stop` → `furi_thread_join`, so it waits for the worker and discards the report rather
than cancelling anything.

## What is not yet confirmed

That these are grinding rather than genuinely stuck. The evidence above is strong but indirect, and
neither poller's display can distinguish the two: Gen2 shows no counter, and USCUID emits
`UscuidUlPollerEventTypeWriteProgress` only after a *successful* page (immediately after
`instance->written++`), so a frozen `147/231` is equally consistent with 84 consecutive failures.

`gen2_poller_write_block_handler` logs `Failed to write block %d` at debug level on every failed block,
so `log debug` during the stall settles it: lines scrolling means grinding, silence means stuck. Worth a
minute before choosing a fix.

## Repro

1. Start a Gen2 / Classic or USCUID-UL clone.
2. Lift the card as soon as the popup switches to "Writing".
3. Wait. The popup does not resolve.

## Why this is an issue rather than a fix in #250

Beyond being a different protocol family from that PR, the two affected pollers need different terminal
events: `gen2`'s `Partial` carries a per-block bitmap, USCUID's carries `pages_written`. And the USCUID
change has the reset caveat described below. It is two changes with two correct answers, in protocols
that PR doesn't touch.

## Direction

There is a working precedent in this codebase. The ISO15693 poller distinguishes "this block failed"
from "the card left" by asking, once, on failure — `iso15693_poller_card_still_present()`, a retried
inventory — and reports `CardLost` instead of blaming the card for blocks it never got the chance to
write. The same check in these two write loops would end the pass immediately and let each report the
terminal event that fits it: `Partial` with the blocks or pages that did land, rather than `Fail`.

Note USCUID has a trap for any fix: its write handler returns `NfcCommandReset` on a failed page
**deliberately**, because a genuine tag can go mute after NAKing a locked page, and re-activation revives
it so the remaining pages still land — turning a total `Fail` into a `Partial`. Any change there has to
tell "reset to revive a NAKing tag" apart from "the card is gone", or it destroys the behaviour that
reset exists to provide.

## Where

`base_pack/nfc_magic/magic/protocols/` — `gen2/gen2_poller.c` (`gen2_poller_write_handler`) and
`uscuid_ul/uscuid_ul_poller.c` (`uscuid_ul_poller_write_handler`). `gen1a` and `gen4` are unaffected;
both were tested.
