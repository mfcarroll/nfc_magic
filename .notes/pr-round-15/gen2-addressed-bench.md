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

## The cards — READ 2026-09-26, and all three differ from the inventory

**Every one is wearing a UID from an earlier test**, so the frames below were generated from what
each card answers NOW, not from `tools/tag-inventory.json`.

| card | UID it wears now | reports | probe used | note |
|---|---|---|---|---|
| `gen-2-card` | `E0 04 01 10 A1 A2 A3 A4` | 28 blocks, IC ref 0x03 | `11223344` | back to 28 -- the 256-block fixture is undone |
| `white-coin` | `E0 07 80 3D E2 E7 3A 29` | 64 blocks, IC ref 0x8B | `55667788` | its own baseline UID; block 8 already clean |
| `black-tag` | `E0 04 01 10 E1 E2 E3 E4` | 70 blocks, IC ref 0x0F | `99AABBCC` | over-claims; blocks 0-1 hold the credential clone |

**A DIFFERENT PROBE VALUE PER CARD**, because `gen-2-card` and `black-tag` both wear `E0 04 01 10`
and a shared probe would make two transcripts indistinguishable.

### What the type lines do NOT say

A first draft of this sheet called `black-tag` and `white-coin` "TI silicon by their UID prefix".
**That is reading a UID-decoded type line as silicon -- the error this round corrected twice**, once
in the release notes and once for `gen-2-card`. `black-tag` presents as NXP SLIX right now because
that is what it is wearing; its baseline capture says `E0 07`, and a baseline is only the first
thing this project saw, not a factory reading.

What is actually known, and it is behavioural rather than decoded:

- **`white-coin` refuses a WRITE BLOCK with OPTION clear** and answers with OPTION set -- the
  measurement the whole OPTION finding rests on. That is TI behaviour observed, not a prefix read.
- **`black-tag` also refuses unaddressed WRITE BLOCK** where `gen-2-card` accepts it, recorded in
  the inventory as the contrast #251 was filed around. Same refusal, so the same reading is
  plausible; it is not the same evidence.
- **`gen-2-card`'s silicon is unknown and the inventory says so in as many words.** It has never had
  a read that predates a write.

So: three CARDS, one chip identified behaviourally, one plausible, one unknown (BENCH-RULE 3).

### Two shelf facts this read settles

- **`gen-2-card` is no longer advertising 256 against 64.** It reports 28 and reads 28, so it was
  re-cloned from the 28-block source and the CFG frame fixed the geometry. The warning at the head
  of NEXT-SESSION is spent.
- **`white-coin` block 8 reads `00 00 00 00`.** The restore written down after the addressed-write
  bench and never confirmed is not outstanding; the whole card is zeros.
- ⚠️ **`gen-2-card` and `SL2S5302` now share `E0 04 01 10 A1 A2 A3 A4`.** One card on the antenna at
  a time, identified by its tape. It is also a neat illustration of why addressing does not close
  #251: a 1-slot inventory cannot tell those two apart.

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

## THE FRAMES, generated 2026-09-26 from the live UIDs

Regenerate rather than copy if a card has moved since: `tools/gen2-addressed-frames.py <uid> --probe <4 bytes>`.

### `gen-2-card`

```
  UID as printed      E0 04 01 10 A1 A2 A3 A4
  on the wire         A4A3A2A1100104E0   (least significant byte first)
  block 0x40 holds    A4A3A2A1   <- restore value, uid[7] uid[6] uid[5] uid[4]

  1  positive control, the form the app sends today -- expect an answer, UID moves
     hf 15 raw -ackw -d 02E0094011223344
     UID should become   E0 04 01 10 44 33 22 11
  2  restore, unaddressed -- always works, no re-address seam to worry about
     hf 15 raw -ackw -d 02E00940A4A3A2A1
  3  THE MEASUREMENT: addressed, correct UID, no OPTION
     hf 15 raw -ackw -d 22E0A4A3A2A1100104E0094011223344
  4  THE DISCRIMINATOR: addressed, UID one byte wrong -- must NOT move the UID
     hf 15 raw -ckw  -d 22E0A4A3A2A1100104E1094011223344
  5  addressed + OPTION, correct UID   (TI silicon wants OPTION on standard writes)
     hf 15 raw -ackw -d 62E0A4A3A2A1100104E0094011223344
  6  unaddressed + OPTION, correct UID
     hf 15 raw -ackw -d 42E0094011223344
  restore after ANY of 3-6 that moved it
     hf 15 raw -ackw -d 02E00940A4A3A2A1
```

### `white-coin`

```
  UID as printed      E0 07 80 3D E2 E7 3A 29
  on the wire         293AE7E23D8007E0   (least significant byte first)
  block 0x40 holds    293AE7E2   <- restore value, uid[7] uid[6] uid[5] uid[4]

  1  positive control, the form the app sends today -- expect an answer, UID moves
     hf 15 raw -ackw -d 02E0094055667788
     UID should become   E0 07 80 3D 88 77 66 55
  2  restore, unaddressed -- always works, no re-address seam to worry about
     hf 15 raw -ackw -d 02E00940293AE7E2
  3  THE MEASUREMENT: addressed, correct UID, no OPTION
     hf 15 raw -ackw -d 22E0293AE7E23D8007E0094055667788
  4  THE DISCRIMINATOR: addressed, UID one byte wrong -- must NOT move the UID
     hf 15 raw -ckw  -d 22E0293AE7E23D8007E1094055667788
  5  addressed + OPTION, correct UID   (TI silicon wants OPTION on standard writes)
     hf 15 raw -ackw -d 62E0293AE7E23D8007E0094055667788
  6  unaddressed + OPTION, correct UID
     hf 15 raw -ackw -d 42E0094055667788
  restore after ANY of 3-6 that moved it
     hf 15 raw -ackw -d 02E00940293AE7E2
```

### `black-tag`

```
  UID as printed      E0 04 01 10 E1 E2 E3 E4
  on the wire         E4E3E2E1100104E0   (least significant byte first)
  block 0x40 holds    E4E3E2E1   <- restore value, uid[7] uid[6] uid[5] uid[4]

  1  positive control, the form the app sends today -- expect an answer, UID moves
     hf 15 raw -ackw -d 02E0094099AABBCC
     UID should become   E0 04 01 10 CC BB AA 99
  2  restore, unaddressed -- always works, no re-address seam to worry about
     hf 15 raw -ackw -d 02E00940E4E3E2E1
  3  THE MEASUREMENT: addressed, correct UID, no OPTION
     hf 15 raw -ackw -d 22E0E4E3E2E1100104E0094099AABBCC
  4  THE DISCRIMINATOR: addressed, UID one byte wrong -- must NOT move the UID
     hf 15 raw -ckw  -d 22E0E4E3E2E1100104E1094099AABBCC
  5  addressed + OPTION, correct UID   (TI silicon wants OPTION on standard writes)
     hf 15 raw -ackw -d 62E0E4E3E2E1100104E0094099AABBCC
  6  unaddressed + OPTION, correct UID
     hf 15 raw -ackw -d 42E0094099AABBCC
  restore after ANY of 3-6 that moved it
     hf 15 raw -ackw -d 02E00940E4E3E2E1
```

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

### `gen-2-card` — 2026-09-26. THE ADDRESSED FORM IS REFUSED. Model (b).

Verbatim (BENCH-RULE 8). Reader after every frame, which is what caught the second finding:

    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   baseline
    hf 15 raw -ackw -d 02E0094011223344                 -> (3) 00 78 F0      unaddressed, no OPTION
    hf 15 reader                                        -> E0 04 01 10 44 33 22 11   MOVED, as predicted
    hf 15 raw -ackw -d 02E00940A4A3A2A1                 -> (3) 00 78 F0      restore
    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   back
    hf 15 raw -ackw -d 22E0A4A3A2A1100104E0094011223344 -> command failed    ADDRESSED, correct UID
    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   UNCHANGED
    hf 15 raw -ckw  -d 22E0A4A3A2A1100104E1094011223344 -> command failed    ADDRESSED, wrong UID
    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   unchanged
    hf 15 raw -ackw -d 62E0A4A3A2A1100104E0094011223344 -> command failed    ADDRESSED + OPTION
    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   unchanged
    hf 15 raw -ackw -d 42E0094011223344                 -> command failed    unaddressed + OPTION
    hf 15 reader                                        -> E0 04 01 10 44 33 22 11   *** MOVED ***
    hf 15 raw -ackw -d 02E00940A4A3A2A1                 -> (3) 00 78 F0      restore
    hf 15 reader                                        -> E0 04 01 10 A1 A2 A3 A4   restored

**THE ADDRESSED FORM DOES NOT WORK AND DOES NOT WRITE.** Refused in both flag combinations with the
CORRECT address, and the UID did not move for either. The card took the same write unaddressed twice
in the same session, either side of the failures, so this is the card refusing the FORM and not a
card that had drifted off the antenna (BENCH-RULE 2b).

The wrong-address frame is uninformative here, which is worth saying rather than counting it: when
the correct address is also refused, a refusal of the wrong one discriminates nothing.

### AND THE SECOND FINDING: a refusal that wrote

`42E0094011223344` -- unaddressed, OPTION set -- **reported `command failed` and the write LANDED.**
The UID moved to `E0 04 01 10 44 33 22 11`.

That is the OPTION acknowledgement cost this round already measured for WRITE BLOCK, appearing on
the PROPRIETARY 0xE0 backdoor and on a card that is not TI. With OPTION set the card owes its answer
only after a standalone EOF, so the frame applies and nothing comes back.

**Had this sheet read the response instead of taking a reader after every frame, that line would have
been recorded as a refusal.** A "command failed" from pm3 is an absence of answer, not evidence that
nothing happened, and on this command the two come apart. The app is already right here for data
blocks -- the read-back exists for exactly this -- but the gen2 backdoor discards its send result
entirely, so nothing in the app depends on it either way.

### What the model now is, and the one frame that would confirm it

Everything fits a front-end that reads the flags byte for OPTION but does NOT honour ADDRESSED for
this command -- so the whole frame is passed to the magic parser, which reads `A4 A3 ...` as
subcommand and reference, does not recognise `0xA4`, and rejects.

**One safe frame separates that from "the parser honours ADDRESSED and rejected for another
reason":** set the ADDRESSED bit but lay the frame out unaddressed, with no UID inserted.

    hf 15 raw -ackw -d 22E0094011223344
    hf 15 reader

- **UID moves** -> the flag's address bit is ignored for `0xE0` entirely. Model confirmed.
- **UID does not move** -> the flag byte is being honoured and something else refuses it.

Same register, same probe, same risk as the frames already run.


## Restore, and record the restore

Every card back to its inventoried UID, confirmed by a final `hf 15 reader` (BENCH-RULE 9).

**Also while `white-coin` is on the reader** -- its block 8 was left holding probe data by the
addressed-write bench and the restore was written down but never confirmed run:

    hf 15 raw -ackw -d 6221293AE7E23D8007E00800000000     zero block 8, addressed + OPTION
    hf 15 raw -ackw -d 2220293AE7E23D8007E008             read it back
