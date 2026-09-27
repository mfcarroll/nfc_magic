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
| ST LRi2K | `lri2k-keychain` | the chip the armed-gen1 wipe hazard was reproduced on, so the stale-address re-check belongs here too |
| NXP SLIX-S 0x02 | `SL2S5302` | completes the three gen1 chips |

Do not write "addressed works" anywhere until these are run. The latch claim was over-scoped from one
chip in round 11 and he caught it; this is the same shape.

## gen-2-card — PASSES, and it was the one that could have killed the design

Currently carrying the `edgedata_64` clone, so UID `E0 04 01 10 D1 D2 D3 D4` and it *presents* as NXP
SLIX. That type line is decoded from the cloned UID, not the silicon; this card's native chip has
never been known, because its UID has always been someone else's.

```
hf 15 raw -ackw -d 2220D4D3D2D1100104E008            addressed READ  blk 8 -> 00 00 00 00 00 77 CF
hf 15 raw -ackw -d 2221D4D3D2D1100104E00811223344    addressed WRITE blk 8 -> 00 78 F0
hf 15 raw -ackw -d 2221D4D3D2D1100104E10855667788    WRONG UID (E0->E1)    -> no answer
hf 15 raw -ack  -d 260100                            -> 00 02 D4 D3 D2 D1 10 01 04 E0
```

Read answers, write is accepted, a one-byte-wrong address is filtered. The inventory's second byte is
`02` -- this card's DSFID -- where the SLIX showed `00`, which is the response format behaving
normally rather than anything about addressing.

**This was the one that could have ended always-addressed.** It is the only gen2 card that takes
unaddressed writes and the one the gen2 path was validated on; had it refused addressed frames, the
fix for TI would have broken the card that already worked. It does not.

Block 8 of the clone is now `11223344`. Nothing depends on it.

## ST LRi2K — accepts, enforces, and stale-address confirmed on a second chip

```
hf 15 raw -ackw -d 222003830050242202E008           addressed READ  blk 8 -> 00 00 00 00 00 77 CF
hf 15 raw -ackw -d 222103830050242202E00811223344   addressed WRITE blk 8 -> 00 78 F0
hf 15 raw -ackw -d 02213800000000                   unaddressed zero blk 56 -> 00 78 F0
hf 15 raw -ck   -d 260100                           -> 00 00 00 00 00 00 24 22 02 E0
hf 15 raw -ckw  -d 222103830050242202E00999AABBCC   addressed, OLD UID    -> no answer
hf 15 raw -ackw -d 02213803830050                   restore blk 56        -> 00 78 F0
hf 15 reader                                        -> E0 02 22 24 50 00 83 03
```

Block 56 holds `uid[7..4]`, which LSB-first is the leading `03 83 00 50`. Zeroing it moves the UID to
`E0 02 22 24 00 00 00 00`, exactly as the SLIX did with its own high half. The pre-write address then
goes unanswered.

**This chip gets enforcement for free from that step** -- the old UID is a wrong UID -- so it needs no
separate mis-addressed control. And it is the chip the armed-gen1 wipe hazard was reproduced on, so
the re-address logic is now measured on the silicon where it will actually fire.

## NXP SLIX-S 0x02 — accepts and enforces

```
hf 15 raw -ackw -d 2220F8350003500204E008           addressed READ  blk 8 -> 00 00 00 00 00 77 CF
hf 15 raw -ackw -d 2221F8350003500204E00811223344   addressed WRITE blk 8 -> 00 78 F0

hf 15 reader                                        -> E0 04 02 50 03 00 35 F8
hf 15 raw -ackw -d 2221F8350003500204E10855667788   WRONG UID (E0->E1)    -> no answer
hf 15 reader                                        -> E0 04 02 50 03 00 35 F8
```

The control was run separately after the gap was spotted, and run better than I specified it:
**bracketed by `hf 15 reader` either side.** That is what makes the silence mean REFUSED rather than
the card having drifted off the antenna -- a negative result needs a positive one around it, or it
is indistinguishable from absence.

## Running total

| chip | card | accepts addressed | enforces address | how enforcement is known | stale-address after UID move |
|---|---|---|---|---|---|
| TI Tag-it HF-I Plus | `white-coin` | yes | **yes** | **read back, distinct data** | n/a -- not gen1 |
| TI Tag-it behaviour | `black-tag` | yes | **yes** | **read back, distinct data** | n/a -- not gen1 |
| NXP ICODE SLIX 0x01 | `slix-1k-50mm` | yes | yes | **read back, distinct data** | **yes** |
| gen-2-card's silicon | `gen-2-card` | yes | yes | **read back, distinct data** | n/a -- not gen1 |
| ST LRi2K | `lri2k-keychain` | yes | yes | silence only | **yes** |
| NXP ICODE SLIX-S 0x02 | `SL2S5302` | yes | yes | **read back, distinct data** | not run |
| unknown | `v2-sticker-50x28` | yes | yes | **read back, distinct data** | n/a -- not gen1 |

**Every chip this app can write accepts addressed WRITE BLOCK.** That is the premise of the whole
approach and it now holds across seven cards, rather than the one it started from.

## ENFORCEMENT IS MEASURED, NOT INFERRED — 2026-09-26, six cards

**Silence is not the same as a non-write**, and nothing on this bench distinguished them until the
gen2 backdoor with OPTION set reported failure and moved the UID anyway. Every enforcement result
above had rested on the card not ANSWERING a wrong address, which was the question that existed when
they were run -- and is a weaker claim than "enforces".

Closed by `tools/enforce-bench.py`, which sends the wrong-address frame with data that DIFFERS from
what the block holds, reads the block BEFORE any restore, and only then sends the right-address
frame -- so a silence is shown to be the address rather than a malformed frame.

| card | flags | wrong address | block after | right address | block after | verdict |
|---|---|---|---|---|---|---|
| `gen-2-card` | `22` | silent | unchanged | `00 78 F0` | `55667788` | ENFORCED |
| `SL2S5302` | `22` | silent | unchanged | `00 78 F0` | `55667788` | ENFORCED |
| `slix-1k-50mm` | `22` | silent | unchanged | `00 78 F0` | `55667788` | ENFORCED |
| `black-tag` | `62` | silent | unchanged | `00 78 F0` | `55667788` | ENFORCED |
| `white-coin` | `62` | silent | unchanged | `00 78 F0` | `55667788` | ENFORCED |
| `v2-sticker-50x28` | `22` | silent | unchanged | `00 78 F0` | -- | ENFORCED |

Transcripts in `enforce-<card>.txt`. Every card restored to what it held and confirmed by a read.

**The flags column is measured too.** The script probes it by writing the block's OWN current value
back with OPTION clear -- a no-op whether accepted or refused -- and reads the answer. It returned
`62` for `black-tag` and `white-coin` and `22` for the rest, independently reproducing the `0x03`
result each of those two gave by hand.

**ALL SEVEN ARE MEASURED.** `lri2k-keychain` was the last on silence alone -- its enforcement came
free from the stale-address result rather than a dedicated frame, which is arguably a better control
(the "wrong" address was one the card had really held) but never read the block back. Run
2026-09-26: `E0 02 22 24 50 00 83 03`, flags `22`, wrong address silent and block 8 unchanged, right
address `00 78 F0` and block 8 `55667788`, restored. **No card is left on an inference.**

### What the sweep of the shelf also turned up

**`slix-1k-50mm` HAS LOST ITS ORIGINAL DATA, and it was never recorded.** Its 2026-09-08 baseline
has `nonzero_blocks: [0, 1]` and a note saying in as many words "this tag is NOT blank. Blocks 0 and
1 hold data" -- the bytes themselves were never captured, because a decode bug ate them and
`original` is write-once. **Both blocks now read `00 00 00 00`.** Lost somewhere between 2026-09-08
and 2026-09-26; the card is `gen1_write: true` and has been cloned and wiped many times since, and
nothing here says which run did it.

Nothing can be done about it now. It is recorded in the inventory so the tag is never re-baselined
as "blank", which it was not.

**And blocks 8 and 9 held probe residue** -- `11 22 33 44` from session A and `99 AA BB CC` from
session B, sitting there since 2026-09-24. Zeroed 2026-09-26 with the addressed form and the whole
card verified 28/28 blank. With 0 and 1 already gone, uniformly blank is the only honest end state:
fragments of old probes read as content to anyone who picks the card up later.

**The old text, kept for the shape of the argument:** Its enforcement came free from the
stale-address result rather than from a dedicated frame: after a write to 56 moved the UID, the
pre-write address got silence and the new one was answered. That is a different and arguably better
control -- the "wrong" address was one the card really had held -- but the block was not read back,
so a silent write is not excluded there the way it now is on the other six.

**CLOSED 2026-09-26 -- all five chips filter on the address.** TI's was the last blank, left on a
reason withdrawn the same day it was given ("it refuses unaddressed writes, so it is already
discriminating on the flag" -- it does not; it refuses writes without the OPTION flag, addressed or
not). Run with the flag SET in both frames, so the address was the only variable left:

    hf 15 raw -ackw -d 6221293AE7E23D8007E108AABBCCDD   WRONG address -> command failed
    hf 15 reader                                        card present throughout
    hf 15 raw -ackw -d 6221293AE7E23D8007E008AABBCCDD   RIGHT address -> (3) 00 78 F0
    hf 15 rdbl -b 8                                     -> AA BB CC DD

The correctly-addressed write is in the set because every failure mode of this probe looks like
silence -- wrong card, moved UID, mangled frame, bad coupling -- and without it the refusal above
proves nothing. Full write-up in [loose-ends-bench.md](loose-ends-bench.md).

## What this settles for the implementation

1. **Always-addressed is safe.** No card refuses it; the card that could have blocked it does not.
2. **The wipe must re-address.** Measured on two chips now, not inferred from the spec: after a write
   to 56 or 57 lands, the UID has moved and the old address gets silence. Re-inventory and re-address
   from the result. At most twice per sweep. 62/63 carry no UID and need nothing.
3. ~~**The gen1 backdoor sequence stays unaddressed.**~~ **SUPERSEDED 2026-09-26 -- it is addressed,
   and "a change with no evidence behind it" was wrong when written.** The wipe's sweep was already
   zeroing 56/57/62/63 through the addressed path, so the evidence was in this round from the start.
   The de-arm probe then added the part nobody had: at block 62 an addressed frame is received,
   parsed and address-filtered on all three gen1 chips, and on the NXP parts it is the ONLY form that
   draws a response. See [dearm-probe-bench.md](dearm-probe-bench.md) and
   [gen1-backdoor-addressed.md](gen1-backdoor-addressed.md).
4. **The SDK cannot do any of this.** `iso15693_3_poller_write_block` hardcodes
   `SUBCARRIER_1 | DATA_RATE_HI` with no flags parameter and no UID, and
   `iso15693_3_write_block_response_parse` is internal to `lib/nfc`. We need our own frame builder and
   our own response check. The app already builds raw frames for the backdoor, so the pattern exists.
5. **An SDK fix is a separate, later PR** -- different repo, needs upstream acceptance. It cannot be
   adopted as a fallback "keyed on API version": a FAP resolves its API imports at LOAD time, so
   naming a symbol the firmware lacks fails the whole load rather than degrading. One binary cannot
   do both. When the call exists in the minimum firmware supported, switch to it and delete the
   workaround.

## What addressing does NOT close in #251

Issue #251 has three parts. Addressing the writes closes one.

- **writes** -- closed by this work.
- **the 1-slot `INVENTORY_T5`** -- still returns whichever tag wins the slot rather than detecting a
  collision. Untouched.
- **no STAY QUIET** -- nothing suppresses a bystander. Untouched.

The issue's worst consequence is the post-wipe UID check being answered by the bystander, printing a
UID belonging to a different card. **That cannot be fixed by addressing**, by construction: the check
exists to discover whether the UID changed, so it cannot be directed at a UID already suspected
stale. It needs the inventory widened or STAY QUIET. Say so when reporting this work, or "addressed
writes" will read as closing #251.
