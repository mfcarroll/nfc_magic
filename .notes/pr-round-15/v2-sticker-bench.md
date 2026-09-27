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

### PHASE 0, 2026-09-26 — read only, nothing written

    hf 15 info    -> UID E0 11 22 33 44 55 66 99
                     TYPE MATCH Emosyn-EM Microelectronics USA
                     SYSINFO 00 0F 99 66 55 44 33 22 11 E0 00 00 3F 03 8B
                     DSFID 0x00   AFI 0x00   IC ref 0x8B   4 bytes x 64 blocks
    hf 15 dump    -> all 64 blocks 00 00 00 00, no locks

**NO GEN3 SIGNATURE IN THE PLAIN READ.** Blocks 20/21 (`0x14`/`0x15`) are `00 00 00 00`, not
`A5 2B 44 2C` / `21 AE 93 00`, and blocks 16/17 (`0x10`/`0x11`) hold no copy of the UID the way the
gold tag's do. The OPTION reads still owe an answer, because a V3 configuration area is defined as
one you read WITH the flag -- but nothing in the plain dump looks like one.

### ⚠️ THE BASELINE IS NOT A FACTORY IDENTITY, AND THE TYPE LINE IS A COSTUME

`E0 11 22 33 44 55 66 99`. Compare `v1-coin-green18`, also from Brian: `E0 11 22 33 44 55 66 91`.
**Both are placeholders from the same hand**, differing in one nibble.

So "Emosyn-EM Microelectronics USA" is decoded from `uid[1] = 0x11` in a value somebody wrote. It is
the same costume `gen-2-card` wears and the same one the v1 coin wore -- the inventory already says
of that card that "its silicon is unknown" for exactly this reason.

**This deflates part of why phase 0 was run first, and that is worth recording rather than quietly
keeping the win.** A pre-write baseline was taken and it was the right thing to do -- it is the state
as received, and nothing can recover that later. But it does NOT give the factory identity, because
the card did not arrive wearing one. The irreplaceable thing was captured; it just turned out to be
worth less than `gen-2-card`'s would have been.

### IT ARRIVED IN PROXMARK'S DEFAULT MAGIC CFG STATE

SYSINFO ends `3F 03 8B`. Those are exactly this app's three gen2 CFG constants:

| | value | meaning |
|---|---|---|
| `ISO15693_MAGIC_V2_CFG_MAXBLOCK` | `0x3F` | 63, so 64 blocks |
| `ISO15693_MAGIC_V2_CFG_BLOCKSIZE` | `0x03` | 4 bytes |
| `ISO15693_MAGIC_V2_CFG_IC_REF` | `0x8B` | the IC reference it reports |

`black-tag` and `white-coin` arrived the same way, which the inventory notes for both. Strong
evidence this is a gen2 magic card, and it settles the question mfcarroll raised:

**THE 64 IS A SETTING, NOT A CAPACITY.** It is the CFG register's `maxblock` read back, so `hf 15
dump` stopping at 64 says only that the card was told to claim 64. The physical size is unmeasured
and a read sweep is the non-destructive way to start.

### THE SWEEP — read only, and `hf 15 dump` cannot substitute

`tools/sweep-read-all.sh` exists for this and its header says why: dump sweeps the ADVERTISED count
and stops, and `probe_capacity` binary-searches so it neither visits every block nor keeps what it
read. Close the interactive client first; the port is exclusive.

    PM3=../proxmark3/pm3 tools/sweep-read-all.sh /dev/cu.usbmodemiceman1 \
        .notes/pr-round-15/v2-sticker-sweep-0-255.txt

**What the sweep can and cannot settle.** Reads answering above 64 do NOT prove more memory -- this
project has both a phantom tail that answers reads past the real top and, on `slix2-gold-30mm`, a
card that answers all 256 addresses because its space ALIASES. So:

- **reads stop at 64** -> consistent with 64 physical, and the claim happens to be honest
- **reads answer past 64** -> either a phantom tail or aliasing, and only a WRITE with an alias check
  separates them: write a distinct pattern high, then read the low block it would alias onto

Nothing is written for either outcome yet. The alias check is phase 1b if the sweep asks for it.

### SWEEP RESULT, 2026-09-26 — the claim is HONEST. All 256 addresses visited.

`v2-sticker-sweep-0-255.txt`, parsed rather than eyeballed:

| blocks | |
|---|---|
| **0-63** | answer, every one `00 00 00 00` |
| **64-255** | refuse, every one |

256 of 256 addresses accounted for, no gaps in the record.

**The advertised count and the physical top agree exactly**, which makes this the first gen2 card on
this shelf where they do: `gen-2-card` has over-claimed and under-claimed at different times, and
`black-tag` currently claims 70 against 64. The CFG is still a programmable setting -- it simply
happens to be set to the truth on this card as received.

**AND THERE IS NO ALIASING.** A hard edge at 64 with 192 consecutive refusals is the opposite of
`slix2-gold-30mm`, which answers all 256 because its space wraps. So **phase 1b is not needed** --
there is no phantom tail to tell apart from an alias, because there is no tail.

**This card SUPPORTS the clone-residue survey's premise.** The survey reads upward from the source
count and stops on an absent run, resting on "past physical capacity a block refuses reads outright".
The gold tag is the known counter-example; this is a fourth gen2 on which the premise holds cleanly.
Worth having, because the survey's design argument is in the reply.

## PHASE 1 FRAMES — from the live UID `E0 11 22 33 44 55 66 99`

`uid[7]` is `0x99`, clear of the `0x09` hazard. Wire address `99665544332211E0`, block `0x40` holds
`99665544`. Probe `A1B2C3D4`, distinct from the other three cards' (`11223344`, `55667788`).
**Every data block is zero, so every restore is zeros and there is no baseline to lose.**

    1  OPTION discriminator -- addressed, OPTION CLEAR, standard write
       hf 15 raw -ackw -d 222199665544332211E008A1B2C3D4
         01 03    wants the flag, like black-tag and white-coin
         00 78 F0 does not, like gen-2-card
    2  addressed, OPTION SET -- should be accepted either way
       hf 15 raw -ackw -d 622199665544332211E008A1B2C3D4
    3  ENFORCEMENT: same flags as whichever of 1/2 was accepted, address one byte wrong
       hf 15 raw -ckw  -d 622199665544332211E108A1B2C3D4      (or 2221... if 1 was accepted)
       hf 15 raw -ackw -d 222099665544332211E008                read block 8 -- did the wrong one write?
    4  restore block 8 to zeros, and confirm
       hf 15 raw -ackw -d 622199665544332211E00800000000
       hf 15 raw -ackw -d 222099665544332211E008

Then the backdoor set from `tools/gen2-addressed-frames.py E0 11 22 33 44 55 66 99 --probe A1B2C3D4`,
which is the same six frames the other three cards ran.

**Read block 8 after the wrong-address frame, before restoring.** On `black-tag` that read was
skipped and the argument had to be made instead of measured; here it costs one frame.

### PHASE 1 RESULT — it does NOT want the OPTION flag, and it takes an addressed write

    hf 15 raw -ackw -d 222199665544332211E008A1B2C3D4   -> (3) 00 78 F0

**Addressed, OPTION CLEAR, accepted.** Two things at once:

- **A sixth card accepting an addressed WRITE BLOCK.** The #251 premise now holds on six cards.
- **It does not want the flag**, like `gen-2-card` and unlike `black-tag` and `white-coin`.

Block 8 now holds `A1 B2 C3 D4` and is restored below.

### THE OPTION REQUIREMENT SPLITS THE GEN2 SHELF 2-2

| card | `2221 <uid> 08 <data>` | |
|---|---|---|
| `black-tag` | `01 03` refused | wants the flag |
| `white-coin` | `01 03` refused | wants the flag |
| `gen-2-card` | `00 78 F0` accepted | does not |
| **`v2-sticker-50x28`** | **`00 78 F0` accepted** | **does not** |

**But that is not two chips against two.** Refusing with `0x03` is a POSITIVE signature -- the tag
naming a bit it needs -- and it groups `black-tag` with `white-coin` for a reason. Accepting is what
almost every ISO15693 part does; it is the absence of a requirement, and an absence groups nothing.
So `gen-2-card` and this sticker are not shown to share silicon by both taking the write, and
nothing here says they do.

**What it does establish is that the round's OPTION handling is exercised by half this shelf**, not
by one outlier. The sticky flag learned from a `0x03` is the path two of four gen2 cards take.

### STILL TO RUN ON THIS CARD

Enforcement, with **different data** so the read can tell whether a wrong address wrote -- reusing
`A1B2C3D4` would make a landed write and a refused one look identical:

    hf 15 raw -ckw  -d 222199665544332211E10855667788   WRONG address, DIFFERENT data
    hf 15 raw -ackw -d 222099665544332211E008           read block 8 BEFORE restoring
      00 A1 B2 C3 D4 ..    the wrong address wrote nothing -- enforcement holds
      00 55 66 77 88 ..    IT WROTE. Stop: this is the result that breaks the round's safety claim
    hf 15 raw -ackw -d 222199665544332211E00800000000   restore to zeros
    hf 15 raw -ackw -d 222099665544332211E008           confirm 00 00 00 00

Then the six backdoor frames already generated, and the four phase-0 OPTION reads at
`0x10`/`0x11`/`0x14`/`0x15`.

### ENFORCEMENT — measured, not inferred. The best control on this shelf.

    hf 15 rdbl -b 8                                     -> A1 B2 C3 D4        before
    hf 15 raw -ackw -d 222099665544332211E008           -> 00 A1 B2 C3 D4 60 3E
    hf 15 raw -ckw  -d 222199665544332211E10855667788   -> command failed     WRONG address
    hf 15 rdbl -b 8                                     -> A1 B2 C3 D4        UNCHANGED
    hf 15 raw -ackw -d 222099665544332211E008           -> 00 A1 B2 C3 D4 60 3E

**Read before AND after the wrong-address frame, with data that differs from what the block held.**
Every other enforcement control here rests on silence, and `white-coin`'s read-back -- the one this
sheet called the strongest -- came after the RIGHT address wrote and used the SAME data in both
frames, so a landed wrong-address write would have been invisible to it. **This is the first direct
measurement on the project that a mis-addressed write changes nothing.** Six cards accept an
addressed write; this is the one where the refusal is proved rather than inferred.

### THE BACKDOOR — gen2 confirmed, and it matches the other three on every frame

    hf 15 reader                                        -> E0 11 22 33 44 55 66 99
    hf 15 raw -ackw -d 02E0094011223344                 -> (3) 00 78 F0    unaddressed
    hf 15 reader                                        -> E0 11 22 33 44 33 22 11   MOVED
    hf 15 raw -ackw -d 02E0094099665544                 -> (3) 00 78 F0    restore
    hf 15 reader                                        -> E0 11 22 33 44 55 66 99   back
    hf 15 raw -ackw -d 22E099665544332211E0094011223344 -> command failed  ADDRESSED, correct UID
    hf 15 reader                                        -> unchanged
    hf 15 raw -ckw  -d 22E099665544332211E1094011223344 -> command failed  ADDRESSED, wrong UID
    hf 15 reader                                        -> unchanged
    hf 15 raw -ackw -d 62E099665544332211E0094011223344 -> command failed  ADDRESSED + OPTION
    hf 15 reader                                        -> unchanged
    hf 15 raw -ackw -d 42E0094011223344                 -> command failed  unaddressed + OPTION
    hf 15 reader                                        -> E0 11 22 33 44 33 22 11   *** MOVED ***
    hf 15 raw -ackw -d 02E0094099665544                 -> (3) 00 78 F0    restore
    hf 15 reader                                        -> E0 11 22 33 44 55 66 99   restored

Probe `11223344` rather than the sheet's `A1B2C3D4`; the restore value `99665544` is unique to this
card so the transcript stays separable. `E0 11 22 33 44 33 22 11` is exactly what `11 22 33 44` into
`uid[7..4]` implies, so the mapping holds on a fourth card.

**gen2 magic, confirmed by a write.** Fourth gen2 card, first in this form factor.

### AND THE OPTION FINDING IS NOT ABOUT TI AT ALL

`42E0...` reported failure and moved the UID here too -- four cards for four. But **this card does
not want the OPTION flag**: it took the standard write with the bit clear. So the acknowledgement
being swallowed is not a property of TI silicon, and not of cards that need the flag. It is what the
OPTION bit does to THIS COMMAND, on every card that has been asked.

Earlier drafts of the gen2 sheet framed it as "on a card that is not TI", which was true and
under-stated. The population is now two cards that require the flag and two that do not, all four
behaving identically when it is set.

### What was NOT touched, on purpose

`0x14`/`0x15` never written, so the gen3 brick hazard was never in play. `0x47`/`0x52` never written,
so the CFG geometry is as received -- the card still claims 64 against a measured 64. Block 8 back
to zeros, confirmed by a read and a dump line.

### STILL OWED IN PHASE 0

    hf 15 raw -ackw -d 422010     READ 0x10, OPTION set
    hf 15 raw -ackw -d 422011     READ 0x11
    hf 15 raw -ackw -d 422014     READ 0x14
    hf 15 raw -ackw -d 422015     READ 0x15

Compare against the plain dump's blocks 16, 17, 20, 21, all of which are `00 00 00 00`. Agreement
closes the gen3 question for this card. A disagreement is the finding the gold tag could never be
given -- a configuration area that only exists behind the OPTION flag.
