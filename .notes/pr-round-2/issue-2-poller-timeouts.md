# NFC Magic: a card removed mid-write is never reported on Gen2/Classic or USCUID-UL — the poller stalls waiting to re-activate

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.

### App

NFC Magic

### App version

2.0

### Describe the bug

Lift the card during a clone and the write never reports anything. The popup stays on
**"Writing / Don't move..."** indefinitely — no "Card removed", no partial result, and no report of the
blocks that *were* written before the card left, which for a half-completed clone is exactly what you
need to know.

Tested across five protocols, lifting the card the moment the popup switched to "Writing":

| protocol | card | needs a re-activation mid-write? | result |
|---|---|---|---|
| **Gen2 / Classic** | CUID magic Classic 1K | **yes** — halts after every block | **no report.** 88s observed, then still nothing |
| **USCUID-UL** | magic NTAG216 | **yes** — resets on a failed page | **no report** after >1 min, frozen at `Writing 147/231` |
| Gen4 | GTU "Ultimate Magic Card" | no — never halts, stays activated | reports immediately |
| Gen1A | magic Classic 1K | no — abandons the write on the first failure | reports promptly |

Gen1A and Gen4 are the useful contrast: the same action produces a proper failure screen there, so this
is not inherent to losing the card mid-write.

On Gen2/Classic there is no escape either, because Back leads to an inescapable loop — a separate
defect, #ISSUE-BACKLOOP. Note Back does not abort a write in any case: the scene's `on_exit` calls
`<proto>_poller_stop` → `furi_thread_join`, so it waits for the worker and discards the report.

### Reproduction

1. Load a saved MIFARE Classic dump (or a MIFARE Ultralight/NTAG dump for USCUID-UL).
2. Start a clone onto a Gen2/CUID magic Classic card, or a magic NTAG that the app detects as USCUID-UL.
3. Lift the card as soon as the popup switches to **"Writing / Don't move..."**.
4. Wait. The popup does not resolve.

### Firmware version

Momentum `mntm-012-308-g8ed809fba`, not Unleashed. Local build carrying unrelated LF RFID changes; none
of them touch `lib/nfc`, `furi_hal_nfc` or `applications/main/nfc`, so the NFC stack is stock Momentum.

### Logs

Gen2 clone, `log debug`, at the moment the card was lifted:

```
9282385 [D][GEN2] Block 34 finished, halting
9282389 [E][ISO14443_3A] Sdd response wrong length
9282492 [D][Nfc] FWT Timeout
9282595 [D][Nfc] FWT Timeout
...            <- every ~103ms for the next 88 seconds, and not one GEN2 line
```

### Anything else?

**Why.** `gen2_poller_write_block_handler` halts the card after every block (`gen2_poller_halt`, at its
end), so reaching the next block needs a **re-activation**. Once the card is gone that activation fails,
the iso3 poller emits `Iso14443_3aPollerEventTypeError`, and `gen2_poller_callback` acts only on
`Ready` — so it discards it and the state machine is never called again. The 103ms cadence above is the
`furi_delay_ms(100)` in `iso14443_3a_poller_run`'s error path.

That single rule covers every protocol tested: **a write stalls exactly when it needs a re-activation
after the card has left.** Gen4 discards the same event and is fine, because it never needs one — it
does not halt between blocks, so it keeps receiving `Ready`, fails the write and sets its terminal
state. USCUID-UL needs one for a different reason: its write handler returns `NfcCommandReset` on a
failed page, deliberately, to revive a tag that went mute after NAKing a locked page.

**Possible direction.** Handle `Iso14443_3aPollerEventTypeError` in the two affected callbacks rather
than discarding it — count it against a budget, and on exhaustion emit the terminal event that fits that
poller: `Partial` with the blocks or pages that did land, rather than `Fail`. There is a precedent in
this pack: the ISO15693 poller added in #250 counts activation failures against
`ISO15693_POLLER_MAX_ACTIVATION_ERRORS` and reports `CardLost`.

Any change to USCUID has a trap: that `NfcCommandReset` is deliberate, and a naive budget would destroy
the behaviour it exists for. It has to tell "reset to revive a NAKing tag" apart from "the card is gone".

**Where.** `base_pack/nfc_magic/magic/protocols/gen2/gen2_poller.c` (`gen2_poller_write_handler`) and
`.../uscuid_ul/uscuid_ul_poller.c` (`uscuid_ul_poller_write_handler`). `gen1a` and `gen4` are
unaffected; both were tested.

Found while testing #250, which does not touch any of these files — `gen2_poller.c`,
`uscuid_ul_poller.c` and the write-check scene are byte-identical to 2.0.
