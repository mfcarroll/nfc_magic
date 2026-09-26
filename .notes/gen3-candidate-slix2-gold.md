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

## Its address space is half what it answers to, and the top half is a mirror

A read of all 256 addresses, 2026-09-26, archived at
`tools/baselines/slix2-gold-30mm_2026-09-26_full-sweep-0-255.txt`.

**Of all 128 candidate periods, exactly one explains every address: 128.** Each period `p` was
tested by asking whether block `b` holds what block `b+p` holds, for every `b`. Every `p` from 1 to
127 gets at least one pair wrong; `p = 128` gets none of its 128 pairs wrong.

A period of 100, say, would require block 16 to match block 116 -- `36 F1 CD 01` against
`00 00 00 00`. (The period is the SPACING between two addresses, not a block number: 100 and 228 are
one cell because they are 128 apart, which is why a period of 100 has nothing to do with them.)

Seven of the agreeing pairs carry distinctive values rather than zeros -- the four factory blocks and
three markers written that day -- so the result does not rest on matching zeros against zeros:

    16  36 F1 CD 01  ==  144        77  AA BB CC DD  ==  205
    17  00 03 48 E0  ==  145        80  DE AD BE EF  ==  208
    20  A5 3B 44 2C  ==  148       100  11 22 33 44  ==  228
    21  21 0F 50 00  ==  149

The card advertises 79, addresses 128, and aliases `addr & 0x7F`.

**This withdraws the 2026-09-08 conclusion**, which said it "answers a read at EVERY address in the
8-bit block space, all 256, so no read-based probe can find its edge". True as stated and
misleading. The top half is a reflection, not memory; the edge is at 128 and IS findable -- but only
by comparing CONTENTS across addresses, which is not what either probe does. Both ask whether a
block answers, and a mirror answers perfectly. The earlier "82 phys" figure was the probe's own
search ceiling and goes with it.

It also settles the question this note raised and could not answer: writing block 100 and reading
block 100 proved persistence, not uniqueness. Block 100 and block 228 are one cell.

**And it is an ALIAS, not a shadow — confirmed from the write side.** The read sweep could not tell
those apart: a card that duplicated its contents into a second region would produce identical reads
at `b` and `b+128` just the same.

    both 100 and 228 read 00 00 00 00
    wrbl 228 FE ED BE EF   ->  228 reads FE ED BE EF, and so does 100
    wrbl 228 00 00 00 00   ->  both read zero again

Writing through the HIGH address changed the LOW one. So there is one cell with two names, and the
high half is not read-only either — a write up there reaches real memory rather than being discarded.

**What that costs the app, on a card like this.** The clone survey reads upward and records the first
and last non-empty block it finds. Here the four factory blocks at 16-21 appear again at 144-149, so
the survey would report a residue range spanning both — "blocks 16-149" — describing 134 addresses
when the distinct data occupies four cells. Nothing here detects a mirror, and nothing reasonably
could without comparing contents across addresses on purpose. Recorded as a limit, not a defect to
fix inside this round.

**Still open:** whether all 128 are physical storage. Blocks 80 and 100 take writes and hold them, so
at least 101 addresses are real; 101-127 read as zero and have never been written.

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

## The capture was complete — the gap was our parser

Raised by mfcarroll: was a dump not taken at inventory? It was, and chasing it reversed the caveat
that stood here.

The baseline records blocks 16, 17 and 20 as UNREADABLE. They were not. The raw capture in
`tools/campaigns/iso15_20260908_012746/raw/` says so in as many words:

    slix2-gold-30mm_baseline_b016  'utf-8' codec can't decode byte 0xf1 in position 475
    slix2-gold-30mm_baseline_b017  'utf-8' codec can't decode byte 0xe0 in position 477
    slix2-gold-30mm_baseline_b020  'utf-8' codec can't decode byte 0xa5 in position 474

`0xF1`, `0xE0` and `0xA5` are bytes 2, 4 and 1 of exactly what those blocks read today --
`36 F1 CD 01`, `00 03 48 E0`, `A5 3B 44 2C`. Block 21 parsed cleanly because `21 0F 50 00` is
entirely below 0x80. **Four blocks, four outcomes, and the split is perfectly predicted by whether
the content is ASCII-range.** pm3 read all four; our parser threw away three.

This is the same defect as the NDEF block-0 case already recorded in the project's history -- fixed
with `errors="replace"` -- but this record predates the fix and was never re-derived.

**So all four blocks held data at first contact, before anything wrote to this card**, and the
factory-state reading rests on four rather than one. That strengthens the indicators. It still does
not make them a classification: everything is a read, and gen3 is a write behaviour.

Their contents on 2026-09-08 remain unrecorded, because the bytes went with the exception, so the
inventory cell stays "+3 unread". What is withdrawn is the inference that they were empty or absent.

`gen-2-card` carries a similar `unreadable_blocks` list and it is NOT this: its campaign contains no
decode errors, so the RF-flakiness attribution in its note stands.
