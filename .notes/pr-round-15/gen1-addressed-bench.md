# The gen1 addressing on hardware — predictions, written before the first frame

For `b312653`. The code is tested and the frames are measured; what is untested is the change
running end to end on a card. **Written and committed before the run** (BENCH-RULE 5), so agreement
means something.

**BENCH-RULE 0 FIRST: close the Flipper CLI and the pm3 client before `./fbt launch`.** Both take
the port exclusively and `runfap.py` waits on it with no timeout, so a blocked install looks exactly
like a broken build. An install that is working takes about two seconds.

## What actually changed on the wire

Two things, and each has its own failure:

1. **The four backdoor frames went from `02 21 <blk> d0 d1 d2 d3` to `22 21 <uid, LSB first> <blk>
   d0 d1 d2 d3`** — or `62` if the card asked for the OPTION flag earlier in the run.
2. **A new inventory between block 56 and block 57**, because 56 moves the UID and 57 has to be
   addressed to where it moved.

## The card that could kill it goes first — `SL2S5302`

BENCH-RULE 7. An ADDRESSED write to block 56 is already measured to land on two of the three chips:
the LRi2K through the wipe's sweep (58/58, the UID moving under it) and an NXP SLIX through the
clone's conversion path. **`SL2S5302` has never had one.** Its 40-block claim keeps the sweep away
from 56 — the reach rule puts L at 47 — so nothing has ever addressed a frame to that register on
SLIX-S silicon. Every backdoor frame it has taken was unaddressed.

So it is the one card here where step 1 could come out the other way, and it goes first.

## First, and isolated from the app: does an ADDRESSED write to block 56 land?

The premise the whole change rests on, isolated from the app (BENCH-RULE 6). Per card, bracketed:

    hf 15 reader                                   record the UID -- call it U
    hf 15 raw -ackw -d 2221<U reversed>38AABBCCDD  addressed write, block 56
    hf 15 reader                                   the UID now
    hf 15 raw -ackw -d 2221<U' reversed>38<orig>   restore, addressed to the MOVED uid
    hf 15 reader                                   back to U

`<U reversed>` is the eight UID bytes least significant first: `hf 15 reader` prints `E0 02 22 24 50
00 83 03`, so the frame carries `03 83 00 50 24 22 02 E0`. Getting it backwards addresses a card that
is not there and the write meets silence — which is a real failure mode, not a typo that shows up as
an error.

**PREDICTION: accepted, `00 78 F0`, and the UID's LAST FOUR printed bytes become `AA BB CC DD`.**
Block 56 carries uid[7..4], and `hf 15 reader` prints uid[0] first, so the tail is what moves. On the
LRi2K that is `E0 02 22 24 AA BB CC DD`.

The restore has to be addressed to the MOVED UID, which is the same seam the Flipper run tests. If the restore
is silent, that is the seam biting in miniature and it is worth saying so before moving on.

**If it is REFUSED on `SL2S5302`:** stop. That would mean SLIX-S takes the backdoor unaddressed and
not addressed, which no chip here has done, and `b312653` would have to become chip-conditional or be
withdrawn. It would also be the single most interesting result of the round.

## Then the whole sequence on the Flipper: does the re-address hold?

    NFC Magic -> the card -> Write UID -> enter the target -> "Not gen2 magic" -> accept gen1

**CHOOSE A TARGET THAT DIFFERS IN BOTH HALVES.** This is the control, and without it the test cannot
fail in the way it is most likely to fail. Block 56 carries uid[7..4] and block 57 carries uid[3..0];
if the target differs from the original only in the low half, a run that writes 56 and loses 57 is
INDISTINGUISHABLE from a run that wrote both. Every UID here starts `E0`, so vary bytes 1-3 as well.

For `E0 02 22 24 50 00 83 03`, `E0 11 22 33 44 55 66 77` differs in seven of eight.

**PREDICTION: Success, and the UID reads back as the whole target**, on all three cards.

**THE FAILURE SIGNATURE IS HALF A UID, NOT SILENCE.** A broken re-address writes 56 and loses 57, so
the card ends up wearing the target's last four printed bytes and its OWN first four: `E0 02 22 24 44
55 66 77`. The app would report Fail, because the verify compares the whole UID. A card that refuses
the addressed form outright is the other outcome and looks different — the UID does not move at all.

Three results, and they say different things:

| after the run | means |
|---|---|
| whole target | the sequence and the re-address both work |
| target's tail, original's head | 56 landed, 57 did not — the re-address is broken |
| unchanged | the addressed form was refused at 56 — step 1 should already have caught it |

## Restore, and record the restore

BENCH-RULE 9. Write each card's original UID back through the same path and confirm with `hf 15
reader`. The originals, recorded before anything is sent, go in this file with the results.

`lri2k-keychain`'s factory UID is `E0 02 22 24 50 00 83 03`, recorded in `tools/tag-inventory.json`
along with the recovery frame. The other two carry fixture UIDs from earlier runs rather than factory
ones, so read them before the first frame rather than trusting anything written down.

## RESULTS

_Not run yet._
