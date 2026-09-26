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

| chip | card | accepts addressed | enforces address | stale-address after UID move |
|---|---|---|---|---|
| TI Tag-it HF-I Plus | `white-coin` | yes | not tested | n/a -- not gen1 |
| NXP ICODE SLIX 0x01 | `slix-1k-50mm` | yes | yes | **yes** |
| gen-2-card's silicon | `gen-2-card` | yes | yes | n/a -- not gen1 |
| ST LRi2K | `lri2k-keychain` | yes | yes | **yes** |
| NXP ICODE SLIX-S 0x02 | `SL2S5302` | yes | yes | not run |

**Every chip this app can write accepts addressed WRITE BLOCK.** That is the premise of the whole
approach and it now holds across all five, rather than the one it started from.

TI's enforcement of the ADDRESS is untested, and the reason given here for not testing it -- "it
refuses unaddressed writes, so it is already discriminating on the flag" -- was withdrawn 2026-09-24.
It does not refuse them; it refuses writes without the OPTION flag, whether addressed or not. See the
correction at the head of [../pr-round-10/unaddressed-write-finding.md](../pr-round-10/unaddressed-write-finding.md).
So four of the five chips are shown to filter on the address and TI is not one of them.

## What this settles for the implementation

1. **Always-addressed is safe.** No card refuses it; the card that could have blocked it does not.
2. **The wipe must re-address.** Measured on two chips now, not inferred from the spec: after a write
   to 56 or 57 lands, the UID has moved and the old address gets silence. Re-inventory and re-address
   from the result. At most twice per sweep. 62/63 carry no UID and need nothing.
3. **The gen1 backdoor sequence stays unaddressed.** It is the magic sequence, measured to work
   unaddressed on all five cards, and addressing it would be a change with no evidence behind it.
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
