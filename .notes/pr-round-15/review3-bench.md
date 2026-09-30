# Review-3 bench — S5, the gen1 data rule, and the converted re-clone

The build is the round-15 tip (`325ae89`, identical in shipped files to every proposed fold, which
move lines between commits without changing the tip -- except S5, which this bench decides). FAP:
`Momentum-Firmware/build/f7-firmware-C/.extapps/nfc_magic_dev.fap`, built clean 2026-09-29.

**Before starting** (BENCH-RULES 0 and "compare against the artifact the device used"): close any CLI
or pm3 client holding the Flipper's port; install the FAP (`FBT_NO_SYNC=1 ./fbt launch
APPSRC=applications_user/nfc_magic_dev` from `Momentum-Firmware`); push `iso15693_identity_64.nfc`,
`iso15693_edgedata_64.nfc`, `iso15693_blocksize8_28.nfc` and `iso15693_wipeseed_64.nfc` from
`tools/test_nfc/`, so the device holds the generator's current output.

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

**1b. The same file onto `slix-1k-50x28` through the gen1 opt-in (optional control).** gen1 cannot
change its 4-byte blocks. Predicted: every 8-byte write refused, a failure screen, no geometry note.
Wrong if any data block is reported written.

Restore `gen-2-card` afterwards as on 2026-09-26 (`reset_2026_09_26` in `tools/tag-inventory.json`):
clone a 64-block image such as `iso15693_wipeseed_64.nfc`, wipe, and Write UID
`E0 00 00 00 00 00 00 00`, leaving 64 blocks of 4 bytes, all zero. It must not be left advertising
8-byte blocks.

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

**2b. `iso15693_edgedata_64.nfc` through the opt-in** (data at 0/1 and 62/63). Predicted:

- "Clone partial", error tone.
- Body: "Cloned 28/60 blocks / Not written: 32 / gen1: 56/57/62/63 differ". 60 is 64 less the four
  registers; the 32 are the blocks past the card's end, all empty.
- Details lists the gen1 line.
- Wrong if it reports success, since the file held data at 62/63.

## The converted re-clone — the UID re-read (A)

**Run it straight after 2a**, with the card still wearing `E0 04 01 10 1D 1D 1D 1D`. Clone
`iso15693_identity_64.nfc` again. The gen2 verify passes on a UID the card already wears, the data
pass writes block 56, the UID follows it, the run converts, repairs the UID, and then re-reads it
behind a field reset. Predicted:

- The same screen as 2a, exactly.
- Afterwards the card answers to `E0 04 01 10 1D 1D 1D 1D` (Info, or `hf 15 reader`).
- Wrong if the screen differs from 2a, or it shows "UID unexpected" / another UID.
- The run is about one field reset longer than 2a; there is nothing else to see.

## Restore

`slix-1k-50x28`: Write UID back to `E0 04 01 50 20 26 06 8C` (gen1 opt-in), then restore its
blocks 0/1 from the pre-bench dump. `gen-2-card`: as in the S5 section. **Do not wipe `slix2-gold-30mm`.**

## Not benched, and why

- **The lift halves of E**: a lift during the survey, and the 100% frame waiting for the re-read.
  The survey and the re-read each take a fraction of a second, so neither can be timed by hand.
  Host-tested: `test_a_card_lifted_during_the_survey_claims_no_size`, and
  `test_the_last_progress_frame_waits_for_the_reread`.
- **The gen1 note on the SUMMARY** ("gen1: 56/57/62/63 are / registers, not data."). It shows only
  when nothing outranks it, which needs a gen1 card reporting 57+ blocks. None here does
  (LRi2K 56, SLIX-S 40, SLIX 28). Host-tested: `test_a_gen1_clone_that_lost_nothing_finishes_with_a_note`.
