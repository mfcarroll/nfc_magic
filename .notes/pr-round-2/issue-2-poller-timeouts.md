# NFC Magic: a card removed mid-write is never reported on Gen2/Classic or USCUID-UL — the poller stalls waiting to re-activate

**Pre-existing.** Found while scoping a change in #250; filing separately because it constrains anything
that touches the shared write scene.

## Observed on hardware

Same procedure each time: start a clone, lift the card the moment the popup switches to "Writing", then
wait.

| protocol | card | needs a re-activation mid-write? | result |
|---|---|---|---|
| **Gen2 / Classic** | CUID magic Classic 1K | **yes** — halts after every block | **no report.** 88s observed, then still nothing |
| **USCUID-UL** | magic NTAG216 | **yes** — returns `NfcCommandReset` on a failed page | **no report** after >1 min, frozen at `Writing 147/231` |
| Gen4 | GTU "Ultimate Magic Card" | no — never halts, stays activated | reports immediately — "Something went wrong while writing" |
| Gen1A | magic Classic 1K | no — abandons the write on the first failure | reports promptly — same screen |
| ISO15693 | gen2 magic | n/a — counts activation errors against a budget | reports |

## Why, confirmed by log

Captured with `log debug` on the Gen2 run, at the moment the card was lifted:

```
9282385 [D][GEN2] Block 34 finished, halting
9282389 [E][ISO14443_3A] Sdd response wrong length
9282492 [D][Nfc] FWT Timeout
9282595 [D][Nfc] FWT Timeout          <- every ~103ms, for the next 88 seconds
...                                      zero GEN2 lines in that whole window
```

The state machine is **stuck, not slow**. Nothing after `Block 34 finished` — no further block attempts,
no failures, nothing. The 103ms cadence is the `furi_delay_ms(100)` in `iso14443_3a_poller_run`'s error
path.

The cause is that `gen2_poller_write_block_handler` **halts the card after every block**
(`gen2_poller_halt`, at its end), so reaching the next block requires a **re-activation**. Once the card
is gone that activation fails, the iso3 poller emits `Iso14443_3aPollerEventTypeError` — and
`gen2_poller_callback` acts only on `Ready`, so it discards it and the state machine is never called
again.

That single rule explains every protocol tested: **a write stalls exactly when it needs a re-activation
after the card has left.** Gen4 and Gen1A never need one mid-write — Gen4 does not halt between blocks
and so stays activated, receives its `Ready`, fails the write and sets its terminal state — which is why
they report promptly despite `gen4_poller_callback` discarding the same event. USCUID-direct needs one
for a different reason: its write handler returns `NfcCommandReset` on a failed page, deliberately, to
revive a tag that went mute after NAKing a locked page.

## Consequence

The write popup never resolves. No "Card removed", no partial result, and no report of the blocks that
*were* written before the card left — which for a half-completed clone is exactly what the user needs.

On Gen2/Classic the escape is worse than merely absent: pressing Back lands in an inescapable loop
between the write-check and write scenes — a separate defect, #ISSUE-BACKLOOP.

Note also that Back never aborts a write in any case. The scene's `on_exit` calls
`<proto>_poller_stop` → `furi_thread_join`, so it waits for the worker and discards the report rather
than cancelling anything.

## Repro

1. Start a Gen2 / Classic or USCUID-UL clone.
2. Lift the card as soon as the popup switches to "Writing".
3. Wait. The popup does not resolve — observed for 88s on Gen2 and over a minute on USCUID-UL, with
   `log debug` showing no state-machine activity at all in that window.

## Why this is an issue rather than a fix in #250

Beyond being a different protocol family from that PR, the two affected pollers need different terminal
events: `gen2`'s `Partial` carries a per-block bitmap, USCUID's carries `pages_written`. And the USCUID
change has the reset caveat described below. It is two changes with two correct answers, in protocols
that PR doesn't touch.

## Direction

Handle `Iso14443_3aPollerEventTypeError` in the two affected callbacks rather than discarding it — count
it against a budget, as the ISO15693 poller does with `ISO15693_POLLER_MAX_ACTIVATION_ERRORS`, and on
exhaustion emit the terminal event that fits that poller: `Partial` with the blocks or pages that did
land, rather than `Fail`.

There is a precedent for the reporting half in this codebase too. The ISO15693 poller distinguishes
"this block failed" from "the card left" by asking once, on failure — `iso15693_poller_card_still_present()`,
a retried inventory — so it can report `CardLost` instead of blaming the card for blocks it never got the
chance to write.

Note USCUID has a trap for any fix: its write handler returns `NfcCommandReset` on a failed page
**deliberately**, because a genuine tag can go mute after NAKing a locked page, and re-activation revives
it so the remaining pages still land — turning a total `Fail` into a `Partial`. Any change there has to
tell "reset to revive a NAKing tag" apart from "the card is gone", or it destroys the behaviour that
reset exists to provide.

## Where

`base_pack/nfc_magic/magic/protocols/` — `gen2/gen2_poller.c` (`gen2_poller_write_handler`) and
`uscuid_ul/uscuid_ul_poller.c` (`uscuid_ul_poller_write_handler`). `gen1a` and `gen4` are unaffected;
both were tested.
