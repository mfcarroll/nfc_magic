# Addressed WRITE BLOCK, measured on ONE chip — 2026-09-24

**Scope: NXP ICODE SLIX, IC ref 0x01, one card.** Not gen1 silicon, not the other four chips this
project writes. The remaining ones are listed at the foot and are not yet run.

`slix-1k-50mm`, UID `E0 04 01 50 20 26 08 63`, freshly wiped. Frames carry the UID LSB-first.
Flags `0x22` = SUBCARRIER_1 | DATA_RATE_HI | T4_ADDRESSED.

Run because the design rested on an inference: that after the sweep writes 56/57 and the UID moves,
an addressed frame carrying the pre-write UID goes unanswered. Spec-level, but this app had never
sent an addressed frame at all, so the composite was untested.

## Session A — is addressing honoured, and enforced?

```
hf 15 raw -ackw -d 222163082620500104E00811223344   correct UID, block 8  -> 00 78 F0   OK
hf 15 raw -ckw  -d 222164082620500104E00855667788   WRONG UID, block 8    -> no answer
hf 15 raw -ck   -d 260100                           -> 00 00 63 08 26 20 50 01 04 E0
```

**Honoured and enforced.** The first addressed frame this project has ever sent is accepted, and a
one-byte-wrong address gets silence rather than a write. Both halves matter: acceptance alone would
not prove the tag was filtering.

## Session B — does a stale address go unanswered?

```
hf 15 raw -ackw -d 02213800000000                   unaddressed, zero block 56   -> 00 78 F0
hf 15 raw -ck   -d 260100                           -> 00 00 00 00 00 00 50 01 04 E0
hf 15 raw -ckw  -d 222163082620500104E00999AABBCC   addressed, OLD UID, block 9  -> no answer
hf 15 raw -ckw  -d 222100000000500104E00999AABBCC   addressed, NEW UID, block 9  -> 00 78 F0
```

**Confirmed.** Zeroing block 56 moves the UID to `E0 04 01 50 00 00 00 00` immediately -- the high
half only, which is what 56 carries. The pre-write address then gets silence, and the same frame at
the new address is accepted. The fourth frame is what makes the third interpretable: the card has not
stopped listening, it is listening as someone else.

Restored: block 56 rewritten with `63 08 26 20`, `hf 15 reader` reads `E0 04 01 50 20 26 08 63`.

## What this settles for the design

Always-addressed is safe for the clone: a gen2 UID lives in a separate register space and a gen1
clone skips 56/57, so the address never goes stale mid-pass.

The WIPE needs re-addressing, and now for a measured reason rather than a spec reading. The sweep
writes 56 then 57, and each changes half the UID, so an address taken at activation is wrong from
block 56 onward. Unhandled, every block above 56 would go unanswered, the absent run would trip, and
the sweep would silently report a shorter card -- on exactly the armed-gen1 path where it currently
reports "Wiped 58/58".

**Re-inventory after a write to 56 or 57 lands, and re-address from the result.** At most twice per
sweep, only on the path where the identity moves. 62/63 do not carry UID and need no re-address.

## NOT YET MEASURED — the other silicon

Addressed WRITE BLOCK is confirmed on exactly two chips across the whole project: TI Tag-it (the
control in the original unaddressed finding, where `hf 15 wrbl` without `--ua` succeeded) and NXP
SLIX here. Three remain, and one of them is load-bearing:

| chip | card | why it matters |
|---|---|---|
| EM-Marin EM4237 | `gen-2-card` | **critical.** The only gen2 card that accepts unaddressed writes, and the one the whole gen2 path was validated on. If it refuses ADDRESSED, always-addressed breaks the card that currently works -- a regression pointing the opposite way from the TI one. |
| ST LRi2K | `lri2k-keychain` | the chip the armed-gen1 wipe hazard was reproduced on, so the stale-address re-check belongs here too |
| NXP SLIX-S 0x02 | `SL2S5302` | completes the three gen1 chips |

Do not write "addressed works" anywhere until these are run. The latch claim was over-scoped from one
chip in round 11 and he caught it; this is the same shape.
