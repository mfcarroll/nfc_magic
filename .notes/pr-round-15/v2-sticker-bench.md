# `v2-sticker-50x28` — baseline first, and it is the last chance to take one

**From Brian**, who wrote proxmark's ISO15693 V3 magic support and sent `v1-coin-green18`. Never
read, never written, never classified. A different form factor from every other gen2 here.
Written and committed BEFORE the first frame (BENCH-RULE 5).

## ⚠️ PHASE 0 IS THE ONLY IRREPLACEABLE PART OF THIS

**Nothing here writes until the baseline is captured and committed.** Every other measurement can be
re-run; a pre-write baseline can be taken exactly once, and this is the only card on the shelf where
the chance still exists.

The precedent is in the inventory, in `gen-2-card`'s own record, and it is worth quoting because it
is what a lost baseline costs:

> THERE IS NONE, and this record exists to say so rather than leave the slot open for a later run to
> fill with a costume. This card was cloned on 2026-07-26, a day before the first instrumented read,
> so no read of it predates a write and its factory identity was never captured.

That card's type line has been misread as its silicon twice in this project, and both times the
correction had to say "its UID has always been someone else's". **Do not create a second one.**

## ⚠️ AND DO NOT WIPE IT, on this bench or in the app

The same shipment carried **known gen3 stickers**. A gen3 card keeps its UID at `0x10`/`0x11` and a
configuration signature at `0x14`/`0x15`, and per 0x6r1an0y -- Brian -- zeroing those on an
un-finalized card **bricks it permanently**. The app's Wipe sweeps every block and would zero both.

- **No frame in this sheet writes `0x14` or `0x15`**, and none writes above block 8 except the gen2
  backdoor registers, which are a separate address space.
- **Do not run the app's Wipe on this card** until phase 0 has settled what it is.
- If the signature IS present, stop and treat it as the gold tag is treated: recorded, not written.

## PHASE 0 — read only. Capture, commit, then continue.

    hf 15 info                    UID, DSFID, AFI, IC ref, advertised blocks, block size
    hf 15 reader                  the type line -- which is a UID decode, not silicon
    hf 15 dump                    every block to the advertised count, saved as JSON

Then the gen3 signature, which is a READ and is how proxmark identifies a V3 non-destructively
(`cmdhf15.c:3362-3373`, and it uses the OPTION flag):

    hf 15 raw -ackw -d 422014     READ block 0x14 (20), unaddressed, OPTION set
    hf 15 raw -ackw -d 422015     READ block 0x15 (21), unaddressed, OPTION set

| `0x14` | `0x15` | means |
|---|---|---|
| `A5 2B 44 2C` | `21 AE 93 00` | **V3, un-finalized** -- STOP, do not write, this is the brickable state |
| `A5 2B 44 2C` | `69 E2 5D 00` | **V3, finalized** -- record it and stop; out of scope for this PR |
| anything else | anything else | not a V3 by proxmark's test -- carry on to phase 1 |

**Compare the plain dump's blocks 20/21 against these OPTION reads.** On an ordinary tag they are the
same memory and will agree. A disagreement is itself the finding -- it is what a configuration area
hiding behind the OPTION flag looks like, and it is what the gold tag could not be tested for.

Also read `0x10`/`0x11` and compare with the UID in the reversed gen3 layout, which is the test the
gold tag failed and then passed on a write:

    hf 15 raw -ackw -d 422010
    hf 15 raw -ackw -d 422011

**Commit the transcript before any write.** That is the whole point of this phase.

## PHASE 1 onward — what this PR actually needs from the card

Only after phase 0 is recorded. Frames from `tools/gen2-addressed-frames.py <live uid>` for the
backdoor, and the standard-write frames built the same way.

1. **Does it want the OPTION flag?** `2221 <uid> 08 <data>` -- addressed, OPTION clear. An error
   `0x03` says yes and identifies it behaviourally, the way `black-tag` and `white-coin` were
   identified. An acceptance says no, like `gen-2-card`. **Read block 8 first and restore after.**
2. **Does it accept an addressed WRITE BLOCK, and does it enforce the address?** Right UID answers,
   one byte wrong is silent, same flags in both so the address is the only variable. This is the
   #251 claim, currently five cards over four identified chips.
3. **Is it gen2 at all?** The backdoor UID write, `02 E0 09 40 <probe>`, then a reader. Reversible
   with the original `uid[7..4]`.
4. **Does the gen2 backdoor refuse the addressed form?** Three cards say yes; this would be a fourth,
   on a card from the developer who writes this tooling.

## PREDICTIONS, committed before the run

- **Phase 0 finds no V3 signature.** Brian labelled it v2 and he is the person least likely to
  mislabel one. If the signature IS there, that is the single most interesting result of the round
  and the write phases do not happen.
- **It accepts an addressed WRITE BLOCK and enforces the address.** Six cards for six; no card here
  has ever failed either.
- **The OPTION question is genuinely open.** `gen-2-card` does not want the flag; `black-tag` and
  `white-coin` do. This card splits the gen2 population 2-2 or 3-1 and either is informative.
- **It is gen2 and its backdoor refuses the addressed form**, like the other three.
- **`v1-coin-green18` is the precedent for expectations**: also from Brian, also arrived as a special
  specimen, and measured identical to a card we already had. A fourth ordinary gen2 is the likely
  outcome and is still worth the frames, because the round's claims are counted in cards.

## What would change what we SHIP

Only two things, and both are unlikely:

- **a card that accepts an addressed write but does NOT enforce the address** -- that breaks the
  safety claim the round is built on, and it is the result the mis-addressed control exists to find.
- **a card whose gen2 backdoor DOES take the addressed form** -- that would reopen the question
  settled tonight and turn a comment fix back into a code change.

Anything else is a fourth data point and a chip count.

## RESULTS

*(phase 0 first, verbatim, committed before anything writes -- BENCH-RULE 8)*
