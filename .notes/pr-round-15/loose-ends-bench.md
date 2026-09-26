# Two loose ends, with predictions written before the frames

Both were left open with a reason that no longer holds. Committed before either is run
(BENCH-RULE 5). **BENCH-RULE 0 first: close the Flipper CLI before pm3 takes the port.**

## Does TI Tag-it enforce the address?

The one blank in the addressed-write table. `white-coin` accepts an addressed write and was never
sent a mis-addressed one, so "four of five chips enforce the address" has a fifth that is UNTESTED
rather than different — and the reply has to say so unless this closes it.

**It was skipped for a reason that was withdrawn the same day it was given**: "TI refuses unaddressed
writes, so it is already discriminating on the flag." It does not. What it refuses is a write without
the OPTION flag, addressed or not.

**THE CARD IS `white-coin`, the coin.** Two cards here carry TI's manufacturer byte `0x07` --
`white-coin` and `black-tag` -- while the two LABELLED `ti-2k-silver-1/2` carry `0x53`, which is not
a registered manufacturer byte at all; that "ti" is the seller's word, not the silicon's. `white-coin`
is where every OPTION measurement was made, so it is also the continuity choice. `hf 15 reader`
should print Texas Instruments and `E0 07 80 3D E2 E7 3A 29`.

**Read it before sending anything, and regenerate if it differs**, because the frames below have that
UID baked in.

    tools/gen1-addressed-frames.py --option-probe E007803DE2E73A29

**THE TRAP, and it is why step 2 is not optional.** Every failure mode of this test looks like
SILENCE: the wrong card on the antenna, a UID that moved since it was recorded, a mangled frame, a
card off the coupling. All of them are indistinguishable from the result the test is looking for.
The correctly-addressed write is what separates them -- if step 2 is ALSO silent, the answer is not
"TI enforces the address", it is "something else is wrong", and step 1 proved nothing. `black-tag`
is the likeliest way to get there, since it is TI too and its UID starts the same three bytes.

**PREDICTION: silence to the wrong address, `00 78 F0` to the right one.** Same result as the other
four chips, and it would make the table complete rather than four-of-five.

**Why the OPTION flag has to be SET in both frames.** Without it this card answers error 0x03 to
anything, so a refusal would be the flag talking and not the address — the frame could not tell the
two apart. With it set, the only variable left is the address.

**And pm3 gets an answer here even though the app cannot.** With OPTION set the card owes its reply
only after a standalone EOF, which the Flipper SDK cannot send — that is the whole reason the app
reads the block back instead. proxmark sends it, and the measured
`6221293AE7E23D8007E00855667788 -> 00 78 F0` on this card proves it. So the response is the
measurement here and the read-back is corroboration, which is the opposite of the app's situation.

## Is `slix2-gold-30mm` gen3, or does it merely hold a copy of its own UID?

Blocks `0x10`/`0x11` READ as this card's UID and `0x14` reads one bit from the V3 config signature.
**All of that is reads, and gen3 is defined by a WRITE** — putting a value into `0x10` and having the
UID follow. Memory holding a UID copy is ordinary and proves nothing.

The baseline makes the mapping exact. From
`tools/baselines/slix2-gold-30mm_2026-09-26_full-sweep-0-255.txt`:

    block 0x10 = 36 F1 CD 01        block 0x11 = 00 03 48 E0
    inventory  = E0 48 03 00 01 CD F1 36

So `0x10` is `uid[7..4]` reversed and `0x11` is `uid[3..0]` reversed — the same shape gen1 uses at
56/57, at different addresses. That makes the result predictable rather than merely interesting.

    hf 15 reader                          expect E0 48 03 00 01 CD F1 36
    hf 15 wrbl --ua -b 16 -d AABBCCDD
    hf 15 reader
    hf 15 wrbl --ua -b 16 -d 36F1CD01     restore
    hf 15 reader                          expect E0 48 03 00 01 CD F1 36

**PREDICTION, and it is a real fork.** If the card is gen3, the UID becomes
`E0 48 03 00 DD CC BB AA` — the tail moving, exactly as block 56 moves a gen1 card's tail. If it is
ordinary memory, the UID does not move at all and block 16 simply holds `AA BB CC DD`.

**Why this is safe, stated explicitly because this card carries a DO-NOT-WIPE.** The hazard is
zeroing `0x14`/`0x15`, which is reported to brick an un-finalized V3 card permanently; those are
blocks 20 and 21 and nothing here touches them. Block 16 is one cell, its previous contents are
recorded above, and the restore is one frame. The card accepts writes at every address, so the
restore cannot be refused.

**What it settles either way.** It is the difference between "no gen3 card exists on either side of
this PR" — which the squash message says — and this bench having the first one. #255 rests on an
attributed report today. It also decides whether the reply's aliasing paragraph should keep calling
the card gen3 TERRITORY or name it.

## RESULTS

_Neither run yet._
