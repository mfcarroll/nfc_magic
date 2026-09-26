# `slix2-gold-30mm` is a gen3 candidate — measured 2026-09-26

Found while looking for a card that could reach one branch of a result-screen string. The card is the
more interesting object.

**The PR's release notes say "No gen3 card exists on either side of this PR."** That is worth
re-examining, but nothing here has established it is false.

## ⚠️ DO NOT WIPE THIS CARD

The app's own release notes carry an attributed report from @0x6r1an0y, who wrote proxmark's ISO15693
V3 magic support: **zeroing blocks 0x14/0x15 on an un-finalized V3 card does not merely clear the
signature, it bricks the card permanently.** Those are blocks 20 and 21, which any wipe reaches in its
first two dozen writes.

Worse here than on an ordinary card: this one accepts a write at every address tried, so the sweep's
absent-run stop can never trip and it would run to the 256-block ceiling or the ten-second budget.
There is no point at which it stops early.

Whether "un-finalized" applies is exactly what is not yet known. Do not resolve it by wiping.

## What this is and is not

**gen3 is defined by a WRITE behaviour: writing 0x10/0x11 changes the card's UID.** Nothing below
tests that. Every observation here is a READ of memory contents, and reads cannot distinguish a
register from ordinary memory that happens to hold the same bytes. Treat this as one card's
indicators pointing somewhere, not as a classification.

## What was measured

    hf 15 info      UID E0 48 03 00 01 CD F1 36, 79 blocks x 4, IC ref 0x01, no tag-info available

`hf 15 dump` over 0..78 is all zeros except four blocks:

    blk 16 (0x10)  36 F1 CD 01
    blk 17 (0x11)  00 03 48 E0
    blk 20 (0x14)  A5 3B 44 2C
    blk 21 (0x15)  21 0F 50 00

**Blocks 16/17 hold the UID, in the reversed layout.** Read, not written -- which is the limit of
what this shows. A copy of the UID in readable memory is not by itself unusual; plenty of products
put one there, and a chip may mirror it read-only. What makes it worth a second look is the ADDRESS
and the BYTE ORDER, not the presence of the value. uid[7..4] at 0x10 and uid[3..0] at 0x11 -- byte
for byte, the same mapping gen1 uses at 56/57, at the addresses this project has documented for gen3
since the capability matrix was written: *"Magic V3 path -- csetuid --v3 (write_block to 0x10/0x11,
reversed) + cfinalize (0x14/0x15)"*.

**Blocks 20/21 are signature-shaped.** Against the V3 config-mode constants the probe carries:

    0x14   expected A5 2B 44 2C    read A5 3B 44 2C    one byte, and one BIT, apart
    0x15   expected 21 AE 93 00    read 21 0F 50 00    first and last byte identical

**"Five of eight bytes" flatters it and should not be quoted that way.** The weight is almost all on
0x14, where three of four bytes are exact and the fourth differs by a single bit. 0x15 matches on
`21` and on `00` -- and `00` is the most common byte on this card by a wide margin, so it carries
close to nothing. One block is suggestive; two are not corroborating each other here.

**And the constants have no recorded provenance.** `V3_SIG_A`/`V3_SIG_B` sit in the probe under a
bare comment with no citation. Whether a near-miss means "a variant" or "not this at all" depends on
whether that value is spec-defined or was observed from one card, and nothing here says which.

**Why the probe said no.** `probe_magictype` sets `v3_config_mode` from `da == V3_SIG_A and db ==
V3_SIG_B` -- exact equality against two constants. A variant or a finalized card fails that test and
is recorded as `gen3_signature: false`, which is then read as "not gen3". The test cannot distinguish
"not a V3 card" from "a V3 card whose signature is not the one constant we know", and the inventory
verdict `unclassified (no write probe run)` has been carrying the first reading.

## And it takes writes past its advertised count

    hf 15 wrbl -b 77  -d AABBCCDD   ok, reads back AA BB CC DD     within the advertised 79
    hf 15 wrbl -b 80  -d DEADBEEF   ok, reads back DE AD BE EF     ABOVE it
    hf 15 wrbl -b 100 -d 11223344   ok, reads back 11 22 33 44     well above it

Combined with the earlier finding that it answers a read at every address in the 8-bit block space,
this card has no discoverable edge by either probe.

**Not yet established: whether those are distinct cells or aliases.** Writing block 100 and re-reading
block 100 proves persistence, not uniqueness -- a mirror at some modulus answers identically. The test
is to write a marker high and read the block it would alias to. Worth doing before any claim that the
card "holds" more than it advertises.

## What this falsifies in shipped code

`iso15693_poller.c`, at the clone's refused-block discriminator:

    // It discriminates because a block past physical capacity refuses reads as well as writes
    // (measured: writes come back Iso15693_3ErrorInternal, reads fail outright).

**Unscoped, and now contradicted by a card on the shelf.** BENCH-RULES rule 3 is the one this breaks:
name the chip, never the family. The same claim is made at the survey and once more near the phantom
-write note.

The behaviour degrades safely, which is why nothing has gone visibly wrong: the discriminator only
ever *suppresses* a capacity claim, so a card that refuses nothing simply never produces one. There is
no false "Card too small". What it does produce is a size note reporting whatever the survey's ceiling
or budget stopped at, on a card whose real capacity is unknown.

## What to do with it — not now, deliberately

1. **Settle it, which needs one write.** Change one byte of block 0x10 and re-inventory. The UID
   moves -> gen3, and that is the only thing that would prove it. It does not move -> those are four
   bytes of ordinary memory holding a UID copy, and the whole reading collapses. Reversible either
   way: the original bytes are in the dump, and the irreversible half of V3 is finalize at
   0x14/0x15, which this never touches. Run the alias test first, since it needs no write at all.
2. **Fix the probe's V3 test** to report the two blocks' contents and their structural match rather
   than a boolean from an equality against one constant.
3. **Re-scope the three shipped comments** to the chips they were measured on.
4. **#255** -- the gen3 pre-flight probe -- stops being hypothetical. It was filed on an attributed
   report with no card behind it, and the argument for it changes if this is a V3 card.

Nothing here changes a result the round has already benched. The clone survey and the guard behave as
measured; this is about what the app would say and do on a class of card it has never met.

## The capture says less than it looks like it does

At the 2026-09-08 baseline, blocks **16, 17 and 20 were UNREADABLE** and 21 was non-zero. Three of
the four blocks this note is about could not be read at all then.

The benign reading, and the likely one: the early probe did single-attempt reads and the project has
already recorded flakiness of exactly this shape on another card. Nothing in this project has ever
written to this tag.

What it costs is that only **block 21's content is confirmed pre-existing** -- it read non-zero
before anything touched the card. For 16, 17 and 20 the first read of any kind is 2026-09-26. That
does not suggest anyone wrote them; it means the factory-state claim rests on one block, not four.
