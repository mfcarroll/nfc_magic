# Loose-end bench 2 — the block-56 address filter, and two unaddressed writes

**Predictions committed BEFORE the run (BENCH-RULE 5).** Two independent things, neither blocking a
push. The device carries `73e9d76`, which is code-identical to the round tip; these are card facts,
not build facts.

## ⚠️ Card state going in

- **`lri2k-keychain` is ARMED** (`0x6996` in block 63, and nothing here clears it). Its UID moves on
  ANY write to 56/57. That is exactly the mechanism under test, so it is fine — but the restore has
  to re-address to the moved UID, which the generated frames already do. Identify it by its tape, not
  by what it answers, and put it back on `E0 02 22 24 50 00 83 03`.
- The three gen1 cards were restored to their own UIDs 2026-09-26. Re-read each with `hf 15 reader`
  before its frames and confirm it matches the sheet; if a card moved, regenerate from what it shows.
- One card on the antenna at a time.

## Item 1 — is block 56 filtered on the address, on each gen1 chip?

**The gap.** The only mis-addressed write ever sent to block 56 (on `SL2S5302`, 2026-09-26) carried
the value the register already held, so a write that LANDED would have read identically to one that
was refused. Enforcement at block 8 is read back on these chips, but block 56 — the register the
whole gen1 addressing exists to keep off a bystander — rests on that one inconclusive frame.

**The fix.** `tools/gen1-addressed-frames.py` now sends the mis-addressed control with DIFFERENT data
(`44 33 22 11`) from the value step 1 writes (`AA BB CC DD`), and reads it back. Block 56 IS the UID
register, so the readout is `hf 15 reader`: a landed wrong write moves the UID.

**PREDICTION, all three chips:** the mis-addressed frame is silent and the UID is UNCHANGED after it
(still `<uid[0:3]> DD CC BB AA`). If instead the UID reads `<uid[0:3]> 11 22 33 44`, the wrong write
landed and that chip does not filter block 56 — stop and record it; it would weaken the gen1-frame
safety claim for that chip, and belongs in the reply.

Run, one card at a time, pasting each transcript under its card:

    tools/gen1-addressed-frames.py E0 02 22 24 50 00 83 03    # lri2k-keychain  (LRi2K, ARMED)
    tools/gen1-addressed-frames.py E0 04 02 50 03 00 35 F8    # SL2S5302        (SLIX-S)
    tools/gen1-addressed-frames.py E0 04 01 50 20 26 06 8C    # slix-1k-50x28   (SLIX)

### lri2k-keychain — LRi2K — TRANSCRIPT

    (paste)

### SL2S5302 — SLIX-S — TRANSCRIPT

    (paste)

### slix-1k-50x28 — SLIX — TRANSCRIPT

    (paste)

## Item 2 — do `black-tag` and the v2 sticker take an UNADDRESSED write?

**Why.** The reply says "nothing measured here requires an addressed write", measured on six of the
seven cards. These two are the gap. Closing it lets the sentence cover all seven directly rather than
by inference.

`hf 15 wrbl --ua` is the unaddressed write. It forces the OPTION flag on for a TI-manufacturer UID
(`black-tag`, `uid[1]=0x07`) and not otherwise (`v2-sticker`, `uid[1]=0x11`) — which is exactly the
flag each card wants, so one command fits both. Block 8 is ordinary memory and reads normally.

**PREDICTION, both cards:** block 8 reads `00 00 00 00` before (both are blank), the unaddressed
write is accepted, block 8 reads `AA BB CC DD` after, and `00 00 00 00` again after the restore. That
is the card taking an unaddressed write. A refusal (block unchanged at `00 00 00 00`) would be the
surprising result and belongs in the record.

    # black-tag  (E0 07 81 B8 AF 14 42 07)  and  v2-sticker-50x28  (E0 11 22 33 44 55 66 99)
    hf 15 reader                          expect the card's UID; brackets the writes
    hf 15 rdbl -b 8                        expect 00 00 00 00
    hf 15 wrbl --ua -b 8 -d AABBCCDD
    hf 15 rdbl -b 8                        PREDICTION: AA BB CC DD  (took the unaddressed write)
    hf 15 wrbl --ua -b 8 -d 00000000      restore
    hf 15 rdbl -b 8                        expect 00 00 00 00
    hf 15 reader                          brackets: the card was there throughout

### black-tag — TRANSCRIPT

    (paste)

### v2-sticker-50x28 — TRANSCRIPT

    (paste)

## Item 3 — housekeeping, while the cards are out

- **`lri2k-keychain` block 8** still holds probe residue from an earlier session. After item 1's
  restore, `hf 15 wrbl --ua -b 8 -d 00000000` and confirm, so the card is left uniformly blank.
- **`gen-2-card`** may still advertise 256 blocks against 64 physical, from the CFG clamp fixture.
  Re-clone any normal 64-block source to reset the geometry, then confirm `hf 15 reader` /
  `hf 15 info` shows 64. Record what it read before and after.
- Update `tools/tag-inventory.json` for anything a run leaves changed (BENCH-RULE 9).

## After the run

If every prediction holds: the block-56 filter is measured on all three gen1 chips, and the
unaddressed-write claim covers all seven cards. I fold both into the reply and the measurement
record, and note the residue/geometry restores in the inventory. Nothing here changes shipped code,
so the fork build does not move for it — only the reply and notes.
