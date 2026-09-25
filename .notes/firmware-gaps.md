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
