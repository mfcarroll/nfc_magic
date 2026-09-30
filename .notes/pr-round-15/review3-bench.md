# Review-3 bench — S5, the gen1 data rule, and the converted re-clone

The build on the device is `/ext/apps/NFC/nfc_magic_dev.fap`, md5 `bc1445910652b67016744d2d96104ab8`,
pulled back and compared 2026-09-29: the FAP built clean at 19:07 from the tree before fold 7. Fold 7
changed three comments and no code, so this is the tip's code; only S5, which this bench decides, could
still change it.

**Done before starting** (BENCH-RULES 0 and "compare against the artifact the device used"):
`iso15693_identity_64.nfc`, `iso15693_edgedata_64.nfc`, `iso15693_blocksize8_28.nfc` and
`iso15693_wipeseed_64.nfc` sent from `tools/test_nfc/` to `/ext/nfc/` with `scripts/storage.py send
-f`, each received back and byte-identical to the repo's copy. The predictions below were made from
those same files.

Every prediction below is written before the run and says what would make it wrong.

## S5 — does any card reach the geometry note on block SIZE alone?

The question behind the one open code decision. `compare_reported_geometry` flags `memory_differs`
when the card's block count OR block size differs from the file's, but the notes print only counts,
so a size-only difference would read "Card still reports 28 blocks" against a file of 28.

**1a. `iso15693_blocksize8_28.nfc` (28 blocks of 8 bytes) onto `gen-2-card`.** The decisive run: the
gen2 CFG frame carries the block size, so this is the one card that could end a success with the
count matching and the size not.

- Record: the result screen, Details if offered, and afterwards `hf 15 info` (block count and SIZE).
- If it ends a clean success with no geometry note, or it fails or ends Partial: no card reaches the
  note on size alone -- **drop the size term**.
- If it ends "Clone finished" with "Card still reports 28 blocks": the term is live -- **keep it and
  print the size** in the note.


```
Clone failed
The UID was written,
but no data block took.
The card has UID only.
```

No details screen. Error tone.

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 01 10 B8 B8 B8 B8
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
[+] SYSINFO... 00 0F B8 B8 B8 B8 10 01 04 E0 00 00 1B 07 01 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x01
[+] Tag memory layout (vendor dependent)
[+]     8 ( or 7 ) bytes/blocks x 28 blocks
[+]     224 total bytes
[usb] pm3 --> 
```

**1b. The same file onto `slix-1k-50x28` through the gen1 opt-in (optional control).** gen1 cannot
change its 4-byte blocks. Predicted: every 8-byte write refused, a failure screen, no geometry note.
Wrong if any data block is reported written.

No details screen. Error tone.

```
Clone failed
The UID was written,
but no data block took.
The card has UID only.
```

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 01 10 B8 B8 B8 B8
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
[+] SYSINFO... 00 0F B8 B8 B8 B8 10 01 04 E0 00 00 1B 03 01 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x01
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 28 blocks
[+]     112 total bytes
[usb] pm3 --> 
```

Restore `gen-2-card` afterwards as on 2026-09-26 (`reset_2026_09_26` in `tools/tag-inventory.json`):
clone a 64-block image such as `iso15693_wipeseed_64.nfc`, wipe, and Write UID
`E0 00 00 00 00 00 00 00`, leaving 64 blocks of 4 bytes, all zero. It must not be left advertising
8-byte blocks.

Restore done.

## The gen1 data rule — mfcarroll's call

On `slix-1k-50x28` (gen1, 28 blocks, IC ref 01, original UID `E0 04 01 50 20 26 06 8C`), in this
order, because the re-clone below needs the card to be wearing 2a's UID.

**2a. `iso15693_identity_64.nfc` through the gen1 opt-in.** 64 blocks, data only in 0/1, nothing at
56/57/62/63. Round 14 reported this "Clone partial ... gen1: 56/57/62/63 differ". Predicted now:

- "Clone finished", success tone, Finish.
- Body: "All data written." then "Card still reports / 28 blocks, IC ref 01." The geometry note
  outranks the gen1 note on the summary, and this card differs on both halves.
- Details: "Didn't fit on the card: 28-55, 58-61", then "gen1: 56/57/62/63 are the UID / backdoor
  registers, not file data.", then the configuration note.
- Wrong if the title says partial, or the tone is the error one.

```
Clone finished
All data written.
Card still reports
28 blocks, IC ref 01.
```

Success tone.

```
Clone notes
- Didn't fit on the card: 29...55 58 59 60 61

- gen1: 56/57/62/63 are the UID / backdoor registers, not file data.

- The file says 64 blocks and IC ref 0F. This card reports 28 blocks and ID ref 01. gen1 cards...
```

**2b. `iso15693_edgedata_64.nfc` through the opt-in** (data at 0/1 and 62/63). Predicted:

- "Clone partial", error tone.
- Body: "Cloned 28/60 blocks / Not written: 32 / gen1: 56/57/62/63 differ". 60 is 64 less the four
  registers; the 32 are the blocks past the card's end, all empty.
- Details lists the gen1 line.
- Wrong if it reports success, since the file held data at 62/63.

Gen1 opt-in accepted. Error tone.

```
Clone partial
Cloned 28/60 blocks
Not written: 32
gen1: 56/57/62/63 differ
```

```
Clone notes
- Didn't fit on the card: 29...55 58 59 60 61

- gen1: 56/57/62/63 are the UID / backdoor registers, not file data.

- The file says 64 blocks and IC ref 0F. This card reports 28 blocks and ID ref 01. gen1 cards...
```

I think that might be a counting bug. I hadn't noticed it before. `Cloned 28/60` isnt' right, the file had 64 blocks so it should be `Cloned 28/64` as the target total comes from the card. I don't remember seeing that error before, so concerned it might be something caused by this round. If it's pre-existing, then we should fix it. If it's this round, we should determine why.

## The converted re-clone — the UID re-read (A)

**Run it straight after 2a**, with the card still wearing `E0 04 01 10 1D 1D 1D 1D`. Clone
`iso15693_identity_64.nfc` again. The gen2 verify passes on a UID the card already wears, the data
pass writes block 56, the UID follows it, the run converts, repairs the UID, and then re-reads it
behind a field reset. Predicted:

- The same screen as 2a, exactly.
- Afterwards the card answers to `E0 04 01 10 1D 1D 1D 1D` (Info, or `hf 15 reader`).
- Wrong if the screen differs from 2a, or it is "Unexpected UID" with another UID.
- The run is about one field reset longer than 2a; there is nothing else to see.

I misread this due to the ordering, and used `iso15693_edgedata_64.nfc` again. I believe the result is still the same. It succeeded, and rescued the UID. Let me know if not and I can repeat 2a then this.

No opt-in. Error tone.

```
Clone partial
Cloned 28/60 blocks
Not written: 32
gen1: 56/57/62/63 differ
```

```
Clone notes
- Didn't fit on the card: 29...55 58 59 60 61

- gen1: 56/57/62/63 are the UID / backdoor registers, not file data.

- The file says 64 blocks and IC ref 0F. This card reports 28 blocks and ID ref 01. gen1 cards...
```

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 01 10 D1 D2 D3 D4
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
[+] SYSINFO... 00 0F D4 D3 D2 D1 10 01 04 E0 02 00 1B 03 01 
[+] DSFID..... 0x02
[+] AFI....... 0x00
[+] IC ref.... 0x01
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 28 blocks
[+]     112 total bytes
[usb] pm3 --> 
```

## Restore

`slix-1k-50x28`: Write UID back to `E0 04 01 50 20 26 06 8C` (gen1 opt-in), then restore its
blocks 0/1 from the pre-bench dump. `gen-2-card`: as in the S5 section. **Do not wipe `slix2-gold-30mm`.**

I first attempted to restore slix-1k-50x28 using the app, but then had to do that manually, which surfaced a bug. I read the tag using the stock NFC app before 1a, and saved it. It saved as type SLIX. The NFC Magic app refused to write saying `This is wrong card` (which is an amusing if cute wording, with the flipper picture). Let's complete this pass and then assess that after.

One other quirk. Not sure if it's a bug. From the Details / clone notes screen, pressing back causes a repeat of the tone that played when the result was first displayed. I'm not sure how the rest of the app handles that. If that's the same in other card types, it's a more general bug in the app and I'll file it separately.

## Reading the results

Read 2026-09-29.

**S5: drop the size term.** In 1a, gen-2-card took the file's geometry through its CFG frame (`hf 15
info` afterwards reports 28 blocks of 8 bytes, IC ref 01) and then refused every 8-byte data write, so
the clone failed before any note could appear. In 1b the gen1 SLIX refused the writes too. Neither run
reached a success whose card differs from the file in block size alone, so `memory_differs` now
compares the block count only (fold 8, at 06, with a host test).

**The gen1 data rule (2a, 2b): as predicted.** 2a finishes with the geometry note on the summary and
the gen1 line in Details; 2b is Partial with "differ".

**The converted re-clone: passes, on `edgedata_64` rather than `identity_64`.** No opt-in, so the gen2
path; the same screen as 2b; and the UID reads back `E0 04 01 10 D1 D2 D3 D4`, so the repair took and
the re-read confirmed it. That exercises the conversion's Partial path instead of its success path,
which is host-tested. No need to repeat it.

**The things raised, checked against the code:**

- **"Cloned 28/60": not new, and deliberate.** The gen1 total has left out the four register blocks
  since the gen1 fallback landed (`20649a5`, 2026-07-30), so "Cloned X/Y" counts the blocks a gen1 card
  can take from the file. This round's own bench recorded it as correct on 2026-09-25
  ([residue-and-geometry.md](residue-and-geometry.md), section 3). The alternative is open -- see
  NEXT-SESSION.
- **The Details list starting at 29: transcription.** Every refused block is marked before it is
  bucketed, and "Not written: 32" is 28-55 plus 58-61, so 28 is in the list.
- **"ID ref": transcription.** Both halves of that note print "IC ref".
- **The tone on Back from Details: app-wide, from upstream.** The Gen2 wipe-partial and USCUID-UL
  partial screens (upstream v2.0, `85a6532`) also play their tone in `on_enter` and open Details as a
  separate scene, so going back replays it there too. Fine to file separately.
- **"This is wrong card" for the stock app's SLIX save: a real gap, as old as the feature**
  (`d3a0a91`, 2026-07-26). File select accepts only `NfcProtocolIso15693_3`, and the stock NFC app saves
  an NXP SLIX tag as `NfcProtocolSlix`. The SDK already hands over a SLIX file's ISO15693-3 data
  through `nfc_device_get_data(dev, NfcProtocolIso15693_3)`, since SLIX's parent protocol is
  ISO15693-3, so the fix is likely the file-select check alone. Deferred until this pass is done.
- **Also noticed: "The card has UID only." (1a)** understates a gen2 card, which also took the file's
  geometry and IC ref. The text dates from 2026-08-04 (`6f85395`).

## SLIX saves as a source

A new build (fold 9, sync point 08): file select takes a save whose protocol is built on ISO15693-3,
so the stock NFC app's SLIX saves are clone sources. The source is mfcarroll's own save of
`slix-1k-50x28`, made with the stock NFC app before 1a (type SLIX), which this app refused with "This is
wrong card".

**S1. That save onto `slix-1k-50x28`, the tag it came from.** Predicted:

- File select accepts it: no "This is wrong card".
- The card already wears the file's UID, so the gen2 verify passes on it, and the file's 28 blocks end
  below block 56, so the data pass never reaches the registers.
- A plain success popup: 28 blocks against a card of 28, IC ref 01 on both sides, nothing above.
- Wrong if the wrong-card screen appears, or the run ends anything but a success.

**Result, 2026-09-29: PASSED.** File select took the save, the clone ended "Success", and a proxmark
read of the tag afterwards matched the save.

**S2, 2026-09-29: PASSED as predicted** (mfcarroll: "S2 passed as expected").

**S2 (optional). The same save onto `gen-2-card`.** Predicted: the gen2 path programs 28 blocks and IC
ref 01, the data pass writes all 28, and the survey finds the card answering reads up to its 64
physical blocks: "Clone finished" with "Card reports 28 blocks, / but holds 64." Restore `gen-2-card`
afterwards as in the S5 section.

## Not benched, and why

- **The lift halves of E**: a lift during the survey, and the 100% frame waiting for the re-read.
  The survey and the re-read each take a fraction of a second, so neither can be timed by hand.
  Host-tested: `test_a_card_lifted_during_the_survey_claims_no_size`, and
  `test_the_last_progress_frame_waits_for_the_reread`.
- **The gen1 note on the SUMMARY** ("gen1: 56/57/62/63 are / registers, not data."). It shows only
  when nothing outranks it, which needs a gen1 card reporting 57+ blocks. None here does
  (LRi2K 56, SLIX-S 40, SLIX 28). Host-tested: `test_a_gen1_clone_that_lost_nothing_finishes_with_a_note`.
