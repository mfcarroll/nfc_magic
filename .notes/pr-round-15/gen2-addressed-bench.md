# Can the gen2 backdoor be addressed? — predictions, then frames

**Raised by mfcarroll 2026-09-26**, reviewing the round: if every other frame set now names the card
it is for, why not this one. Written and committed BEFORE the first frame (BENCH-RULE 5).

## The question, and why the standing answer is not good enough

`iso15693_poller_build_gen2_frame` sends `02 E0 09 <ref> d0 d1 d2 d3` -- flags `0x02`, unaddressed.
The reason given at `ISO15693_MAGIC_FLAGS`, again at WHAT REMAINS UNADDRESSED, and in the 2.3 notes
is that `0xE0` is proprietary, so **"a conforming tag rejects it on the command and there is no
standard frame for a bystander to take."**

That argument covers conforming tags. **Another gen2 magic card in the field parses `0xE0 0x09`
exactly as the target does** -- and the sequence writes the CFG register as well as the UID, so a
bystander gen2 card would come away with a different block count, block size and IC reference on top
of a different identity. The population carrying two magic ISO15693 cards is exactly this app's user.

Whether the backdoor honours the ISO15693 ADDRESSED flag is not a question the standard answers. The
backdoor is not a conforming implementation; only the silicon says.

## BENCH-RULE 0 FIRST

**Close the Flipper CLI and any other pm3 session** before the first frame. The port is exclusive and
a busy port looks exactly like a card that refuses everything.

## The cards — three, and two are TI silicon

| card | UID as inventoried | chip from the UID | note |
|---|---|---|---|
| `black-tag` | `E0 07 81 B8 AF 14 42 07` | `E0 07` = Texas Instruments | 64 blocks |
| `white-coin` | `E0 07 80 3D E2 E7 3A 29` | `E0 07` = Texas Instruments | 64 blocks, TI Tag-it HF-I Plus |
| `gen-2-card` | **READ IT FIRST** | unknown -- only ever worn a cloned UID | left advertising 256 blocks against 64 physical |

**`gen-2-card`'s UID is whatever was last cloned onto it, so read it before generating frames.**
Identify each card by its tape, not by what it answers.

## ⚠️ THE HAZARD, and the pre-flight that bounds it

If the backdoor parser IGNORES the flag byte and reads positionally, `22 E0 <uid...>` is read as
`cmd=E0, sub=uid[7], ref=uid[6], data=uid[5..2]` -- a write into an arbitrary register, because the
UID travels least significant byte first and `uid[7]` is the LAST byte `hf 15 reader` prints.

**It is safe only while `uid[7]` is not `0x09`.** `tools/gen2-addressed-frames.py` refuses to emit
addressed frames when it is. Both cards above end `07` and `29`, so both are clear; check
`gen-2-card` when its UID is known.

**Only block `0x40` (uid[7..4]) is written.** The CFG registers `0x47` and `0x52` are NOT touched:
a wrong value there changes what the card reports about itself, and one card on this shelf is
already carrying a bad geometry from an earlier fixture.

## The sequence, per card

Frames from `tools/gen2-addressed-frames.py <uid>`, whose selftest asserts the wire order against the
gen1 transcript and the `uid[7..4]` mapping against the gold tag's measured UID move. Hand-typing
these is the error the tool exists to remove.

    1  hf 15 reader                        baseline UID
    2  hf 15 info                          blocks / IC ref / DSFID, so a change elsewhere shows
    3  UNADDRESSED write 0x40 = AABBCCDD   POSITIVE CONTROL -- the card takes the backdoor NOW
    4  hf 15 reader                        UID must have moved to ...DD CC BB AA
    5  UNADDRESSED restore 0x40            back to the original
    6  hf 15 reader                        confirm the restore
    7  ADDRESSED, CORRECT UID              *** THE MEASUREMENT ***
    8  hf 15 reader                        did it move?
    9  restore if it moved
    10 ADDRESSED, UID ONE BYTE WRONG       *** THE DISCRIMINATOR -- must NOT move ***
    11 hf 15 reader                        confirm it did not
    12 UNADDRESSED write again             CLOSING BRACKET -- the card was there throughout
    13 restore, and confirm with a reader

**3-6 and 12 are the bracket (BENCH-RULE 2b).** Without them a silence at 7 means nothing: a card
that has drifted off the antenna is indistinguishable from one refusing the addressed form.

**10 is the control that can fail (BENCH-RULE 2).** An acceptance at 7 alone does not show the card
MATCHED the address -- a card ignoring the flag entirely answers identically.

Then 5 and 6 from the tool, **addressed+OPTION and unaddressed+OPTION**, on the TI cards. The app
sends the gen2 backdoor with OPTION clear and gen2 cloning works on TI silicon, so `0xE0` evidently
needs no OPTION where `0x21` does -- worth confirming rather than assuming, since the round's whole
OPTION finding came from one card wanting a flag nobody expected.

## PREDICTIONS, committed before the run

**Model (a) -- the parser honours the ADDRESSED flag.** 7 accepted and the UID moves; 10 silent and
the UID does not.
⇒ **gen2 CAN be addressed.** The release-notes and comment claim "cannot usefully be otherwise" is
false, the fix is a flags byte and a UID in the frame builder, and it belongs in this round beside
the other three frame sets.

**Model (b) -- the parser ignores the flag and reads positionally.** 7 refused or silent and the UID
does not move, because `uid[7]` is not `0x09`; 10 the same.
⇒ **gen2 cannot be addressed this way.** The current behaviour stands, but the REASON in the code is
still wrong -- it would be "the backdoor's parser has no address field", which is a measured fact,
not "a conforming tag rejects it", which is an argument about the wrong population.

**Model (c) -- the parser ignores the flag byte but finds its own fields.** Both 7 AND 10 accepted,
the UID moving each time.
⇒ **THE WORST OUTCOME, and the one to stop on.** It means an address on these frames would be
decorative: a bystander gen2 card takes them whatever we put in the flags. Record it and do NOT
"address" the sequence, because shipping a frame that looks addressed and is not is worse than the
honest unaddressed one.

**Which is most likely:** (b). The magic write is a bolt-on to a standard stack and the four-byte
payload at a fixed offset suggests a parser that switches on `0xE0` and reads from there. But (a) is
cheap to rule in and the round has already had one "this is unmeasured" turn out to be measured.

**The result does not depend on the model being right.** What the UID does after 7 and after 10 is
the whole measurement; the models only say what to conclude.

## RESULTS

*(fill in per card -- transcripts verbatim, BENCH-RULE 8)*

## Restore, and record the restore

Every card back to its inventoried UID, confirmed by a final `hf 15 reader` (BENCH-RULE 9).

**Also while `white-coin` is on the reader** -- its block 8 was left holding probe data by the
addressed-write bench and the restore was written down but never confirmed run:

    hf 15 raw -ackw -d 6221293AE7E23D8007E00800000000     zero block 8, addressed + OPTION
    hf 15 raw -ackw -d 2220293AE7E23D8007E008             read it back
