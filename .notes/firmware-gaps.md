# Firmware gaps found by this PR — for an upstream issue, not for this repo

Two defects in the Flipper firmware's `lib/nfc` ISO15693 support, both found while making the NFC
Magic app write a TI Tag-it HF-I Plus. Neither is fixable from an application, and together they make
one class of card unwritable by any FAP.

**Nothing here blocks this PR.** The app ships a workaround that needs no firmware change; see the end.
This file exists so the upstream report can be written from measurements rather than from memory.

Line numbers are Momentum 87.15. The whole `lib/nfc/protocols/iso15693_3/` directory is byte-identical
across Momentum, Unleashed, RogueMaster, Xero and official, so this is not a fork-specific problem.

## Gap 1 — `iso15693_3_poller_write_block` hardcodes its request flags

`iso15693_3_poller_i.c:248`

```c
bit_buffer_append_byte(
    instance->tx_buffer, ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI);
```

No flags parameter and no UID, so the only WRITE BLOCK the SDK can send is **unaddressed, with the
OPTION flag clear**. Two consequences:

- **Every write reaches every tag in the field.** A second card in range takes it. On ISO15693 the
  operating range makes that ordinary rather than exotic -- a wallet or a badge holder is enough.
- **Silicon that requires the OPTION flag cannot be written at all.** TI Tag-it HF-I Plus is one such
  card; see the measurements below.

`iso15693_3_write_block_response_parse` is declared in `iso15693_3_i.h`, which is internal to
`lib/nfc`, so an app building its own frame must also reimplement the response decode.

The same shape applies to `iso15693_3_poller_read_block`, `..._write_afi` and `..._write_dsfid`.

## Gap 2 — the poller cannot send a standalone EOF

`furi_hal_nfc_iso15693.c:155-178`. `iso15693_3_poller_encode_frame` writes a SOF, then the data, then
one EOF, and `furi_hal_nfc_iso15693_poller_tx` is the only transmit path. There is no way to emit an
EOF on its own.

ISO/IEC 15693-3 §10.3.1: with the OPTION flag set, the VICC returns its response to a Write command
**only after it receives an EOF from the VCD**. So even with Gap 1 fixed, an OPTION write's
acknowledgement is unreachable: the card programs the block and waits for a signal the reader has no
way to produce.

proxmark3 does produce it -- `CodeIso15693AsReaderEOF` / `SendDataTagEOF`, `armsrc/iso15693.c:188`
and `:2004`, dispatched at `:3053` when a raw command gets no response. That is the only reason
`hf 15 wrbl` works on this card.

The listener side of the Flipper HAL already has the equivalent shape in `nfc_iso15693_listener_tx_sof`
(`lib/nfc/nfc.h:381`); the poller has no counterpart.

**The two gaps are one change.** Fixing 1 alone yields a write that works and an answer that never
comes, which is precisely what this app then had to work around.

## The measurements

All on `white-coin`, TI Tag-it HF-I Plus, IC ref 0x8B, UID `E0 07 80 3D E2 E7 3A 29`, with a
proxmark3, each result bracketed by `hf 15 reader` and read back from the card.

| flags | addressed | option | result |
|---|---|---|---|
| `0x02` | no | no | refused, error 0x01 |
| `0x22` | yes | no | refused, error 0x03, "the option is not supported" |
| `0x42` | no | **yes** | **accepted** |
| `0x62` | yes | **yes** | **accepted** |

`0x02` is what the SDK sends. The OPTION flag is necessary and sufficient on this chip; the addressing
is orthogonal to it.

On the device, with the app sending `0x62`, the Flipper log shows the flag being set and then **216
consecutive `FWT Timeout`** -- 72 blocks at three attempts, every write unanswered, with no timeout
for any of the reads interleaved with them. The card was nonetheless fully written: four distinct
patterns at blocks 0, 8, 32 and 63 were present before the wipe and all 64 blocks read back as zeros
after it, while the app reported that nothing had been cleared.

## ⭐ A possible app-side route to Gap 2 — UNTESTED, raised by mfcarroll

The read-back exists because we cannot send the standalone EOF an OPTION write's response waits for.
That may be wrong, and the test is cheap.

`iso15693_3_poller_send_frame` appends the CRC unconditionally, so an empty buffer through it is
SOF + 2 CRC bytes + EOF -- a malformed short frame, not an EOF. **But the CRC is added by the
`iso15693_3` layer, not the HAL.** `nfc_poller_trx` takes an already-CRC'd buffer and the HAL only
does the 1-of-4 encoding, so calling it DIRECTLY with an empty buffer emits **SOF + EOF and nothing
else**. Its contract -- "Must ONLY be used inside the callback function" -- is satisfied: the write
path runs inside the poller callback. The only plumbing needed is keeping the `Nfc*` that
`iso15693_poller_alloc` is already handed and currently drops.

**Why it is still a question rather than a fix.** proxmark's `CodeIso15693AsReaderEOF` emits the EOF
symbol ALONE. Ours would be SOF then EOF. Whether a card waiting for an EOF accepts that, or treats
the SOF as a new request and resets, is silicon-dependent and the spec does not settle it.

**The test:** store the `Nfc*`, send the empty frame immediately after an OPTION write, and see
whether a response arrives instead of a timeout. One run on a TI Tag-it answers it.

**If it works** the acknowledgement comes back directly, the read-back is dead on that path, and a
write costs one frame instead of a write plus a read -- and Gap 2 stops being a firmware dependency
at all, leaving only Gap 1 upstream. **If it does not**, nothing changes.

Not a blocker either way: the read-back is measured and safe.

### RUN 2026-09-26 — it does NOT work, and the failure is the predicted one

Branch `experiment-eof-frame`, wipe of a TI Tag-it, 72 blocks. Every probe returned `trx=2` --
`NfcErrorTimeout` -- with no exception across 88 attempts. The card did not answer SOF + EOF.

**A caveat about the log, because the line is misleading.** It also printed `rx=2 bytes` every time,
which is NOT a response. `nfc_poller_trx` does not reset `rx_buffer`; it fills it only on success and
breaks out before touching it on the timeout path. Those two bytes are stale -- almost certainly the
`01 03` OPTION refusal from the head of the run, which is the last thing that buffer legitimately
held. The `trx` code is the only signal in that line. Uniform across every block, so the result
itself is clean.

**This does not condemn the EOF approach, only SOF+EOF.** Which is exactly the ambiguity recorded
above: proxmark emits the EOF symbol ALONE, ours prepends a SOF, and a card reading that SOF as the
start of a fresh request would answer nothing -- which is what happened.

The run also re-confirmed the read-back: blocks 0-63 each needed ONE attempt (write times out, probe
times out, read-back succeeds, break) while 64-71 burned all three, and the wipe reported 64 cleared
of 72 attempted. Correct on both counts.

### THE STANDALONE EOF WORKS — measured 2026-09-26, firmware branch `iso15693-poller-tx-eof`

The change was written, the firmware flashed, and a TI Tag-it wiped. **Every EOF collected the
response the poller could not previously get at all**, and the bytes say what they should:

    blocks 0-63   trx=0  rx=3  [ 00 78 F0 ]        flags 00, no error, then CRC
    blocks 64-71  trx=0  rx=4  [ 01 10 1E 06 ]     error flag, code 0x10, then CRC

**Both corroborate against records made weeks earlier and independently.** `00 78 F0` is byte for
byte the successful WRITE BLOCK response proxmark printed in
[pr-round-15/addressed-writes-measured.md](pr-round-15/addressed-writes-measured.md), three separate
times. `0x10` is "block not available", already in this file's own poller comments from the LRi2K
work — and it arrives on exactly the eight blocks past this card's 64-block capacity and on no
others. A byte count could not have shown either.

So the acknowledgement is real, it is correct, and it distinguishes a landed write from a refused one
without touching the card's memory. **That is Gap 2 closed at the source rather than worked around.**

### It is not 100% per attempt, and that matters for any implementation

**9 of 96 probes timed out** (blocks 7, 16, 17, 18, 30, 32, 34, 36, 38). All but one answered on the
retry the loop already performs; block 7's was settled by the read-back instead. Roughly a tenth,
which is ordinary coupling flakiness rather than anything structural — the same marginality that
shows up everywhere else on this bench.

Worth writing down because it decides the shape of a real implementation: **an EOF can be lost like
any other frame, so it needs the same retry discipline as the write**, and a first version that
treats a single EOF timeout as "the write failed" would be wrong about one block in ten.

### What it does NOT establish

**One card, one chip.** TI Tag-it HF-I Plus. Whether other silicon answers a bare EOF the same way is
untested, and this project has twice shipped a claim scoped to a family from a single card. Do not
write "cards answer a standalone EOF" anywhere.

**And that cannot be widened with the cards here — do not try, it looks testable and is not.** I
proposed re-running on the LRi2K and the SLIX as "just a card swap". Both produced no EOF lines at
all, which reads like a broken build and is not: the probe is guarded on a write TIMING OUT and on
`write_option`, and `write_option` is set only by an `0x03` refusal. Those chips take writes without
the OPTION flag and acknowledge them normally, so neither condition is ever met.

That is the mechanism rather than the probe. **A standalone EOF only has something to collect when
the card is holding a response pending, which only happens with OPTION set.** On a card that answers
its writes directly there is nothing for an EOF to fetch. `white-coin` and `black-tag` are both TI
Tag-it, so the bench holds exactly one chip that can exercise this, and widening the claim needs
silicon nobody here owns.

**And it cannot be used by this PR.** It needs API 87.2, and the PR targets stock firmware. The
read-back stays in round 15 regardless; this makes it removable later, upstream willing.

### And a real EOF is a SIX-LINE change, which the run also established

`iso15693_3_poller_encode_frame` in `targets/f7/furi_hal/furi_hal_nfc_iso15693.c` writes the SOF and
EOF as literal bytes of the 1-of-4 stream:

    frame_buf[0] = 0x21;            // SOF
    ...
    frame_buf[byte_pos++] = 0x04;   // EOF

So a standalone EOF is a one-byte frame, `0x04`, through `furi_hal_nfc_poller_tx_common` -- the same
call the poller already uses, which clears the FIFO, disables parity and transmits the bits as given.
**No transparent mode and no bit-banging**, which is what proxmark needs because it drives the field
itself. The pieces:

1. `furi_hal_nfc_iso15693_poller_tx_eof()` -- emit `0x04`, 8 bits, via `poller_tx_common`. Mirrors
   `furi_hal_nfc_iso15693_listener_tx_sof()`, which is a two-line function.
2. An `Nfc` level call beside `nfc_iso15693_listener_tx_sof()`. To be useful it wants a trx shape --
   send the EOF, then run `nfc_poller_trx_state_machine` with the FWT so the answer is collected.
3. The app calls it after an OPTION write instead of reading the block back.

Still silicon-dependent, and still unanswered: whether a TI Tag-it accepts a bare EOF. The point is
that finding out is now cheap.

## What a fix looks like

1. Give `iso15693_3_poller_write_block` a flags argument and an optional UID, or add an addressed
   variant beside it. Export the response parser, or return the raw response.
2. Add a poller-side EOF transmit -- `furi_hal_nfc_iso15693_poller_tx_eof` and an `Nfc` level call,
   mirroring `nfc_iso15693_listener_tx_sof` -- and have the write path use it when OPTION is set.

## What this app does instead, and why it stays

It builds its own addressed WRITE BLOCK frame, reimplements the response decode, learns the OPTION
requirement from the card's own 0x03 refusal, and -- because the acknowledgement is then unreachable --
**reads the block back and compares it** rather than treating silence as a refusal.

That workaround is not removable by an upstream fix. The app has to keep working on today's firmware,
so a fixed SDK would become a fast path with this as the fallback, keyed on API version.

## Separate project — a proxmark3 client bug found the same way

Not a Flipper issue and not this PR's, but found here and worth filing against proxmark3.

`client/src/cmdhf15.c:3126` (and the same block in `restore` at `:3211` and `wipe` at `:4260`) decides
whether to set the OPTION flag on a write from the UID's manufacturer byte:

```c
// TI needs OPTION
if (uid[7] == 0xE0 && uid[6] == 0x07) {
    add_option = true;
}
```

**On a magic card that byte is whatever was last written to it.** `black-tag` is TI silicon carrying a
cloned `E0 04 01 10 E1 E2 E3 E4`, so pm3 reads "NXP", withholds the flag, and `hf 15 wrbl` fails with
error 0x01 on a card it can write perfectly well. Sending the same frame by hand with 0x40 set works:

```
hf 15 wrbl -b 0 -d AABBCCEE                       -> error 1, "The command is not supported"
hf 15 raw -ackw -d 6221E4E3E2E1100104E000AABBCCEE -> lands; rdbl reads AA BB CC EE
```

The consequence is worse than a failed write, because the failure looks like a locked block: three
different values were refused at block 0 while block 8 took one, which reads as memory protection
rather than a missing flag.

**It is the same error this app nearly shipped** -- deriving a property of the silicon from a UID that
the tool itself rewrites. Fixed here by keying on the card's own refusal instead; see
ISO15693_POLLER_OPTION_FLAG. A UID-independent fix for pm3 would be the same shape: try without, and
let the card's answer decide.

## Where this should be filed

The two `lib/nfc` gaps go to the firmware repo, not `xMasterX/all-the-plugins` -- they are SDK defects,
not plugin ones, and the plugin pack carries no copy of the SDK. The pm3 client bug goes to
`RfidResearchGroup/proxmark3`. None filed yet.
