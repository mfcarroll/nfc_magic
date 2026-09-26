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

**Do not build these by hand.** `tools/gen1-addressed-frames.py <uid as hf 15 reader prints it>`
emits all four lines with the predictions beside them, and asserts its byte order against a frame a
card actually accepted before it prints anything.

Two reversals are involved and they are not the same reversal. The ADDRESS is the eight UID bytes
least significant first, so `E0 02 22 24 50 00 83 03` travels as `03 83 00 50 24 22 02 E0`. Block
56's DATA is uid[7],uid[6],uid[5],uid[4] — so restoring `...50 00 83 03` takes `03 83 00 50`, which
is the printed tail reversed, not the printed tail. The script had that backwards on its first run.

Neither mistake fails loudly. A reversed address names a card that is not in the field, so the write
meets silence and reads exactly like the refusal this bench is looking for.

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

For `E0 02 22 24 50 00 83 03`, `E0 11 22 33 44 55 66 77` differs in both halves -- which is the
property that matters. It differs in six of eight bytes, not seven: byte 0 is `E0` on every ISO15693
tag and byte 2 is `22` in both by coincidence. Count bytes and the number moves with the next card;
count halves and it cannot.

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

### pm3 — `SL2S5302`, NXP ICODE SLIX-S, 2026-09-26

**The card that could have killed the change, and it goes the other way.** Identified by mfcarroll,
not by the transcript: `E0 04 01 10 A1 A2 A3 A4` is `tools/test_nfc/iso15693_slix_28.nfc`'s UID, so
the `TYPE MATCH ... SLIX` line describes the costume rather than the silicon and more than one card
here wears it.

    hf 15 reader                                    -> E0 04 01 10 A1 A2 A3 A4
    hf 15 raw -ackw -d 2221A4A3A2A1100104E038AABBCCDD -> (3) 00 78 F0
    hf 15 reader                                    -> E0 04 01 10 DD CC BB AA
    hf 15 raw -ackw -d 2221AABBCCDD100104E138AABBCCDD -> command failed
    hf 15 reader                                    -> E0 04 01 10 DD CC BB AA
    hf 15 raw -ackw -d 2221AABBCCDD100104E038A4A3A2A1 -> (3) 00 78 F0
    hf 15 reader                                    -> E0 04 01 10 A1 A2 A3 A4

**Every prediction, exactly.** The predictions were committed in `73e9d76` before the first frame,
so this is a test rather than an explanation.

- **An ADDRESSED write to block 56 is accepted** — `00 78 F0`, the same answer the unaddressed form
  gets on this chip.
- **The UID moved to precisely the predicted value.** `AA BB CC DD` into block 56 comes back as the
  UID's last four printed bytes REVERSED, `DD CC BB AA`, which is the `uid[7..4]` mapping
  `iso15693_poller_predict_uid` computes. The value the app would have predicted is the value the
  card produced.
- **The address is ENFORCED.** One byte wrong — `E0` to `E1` in the last wire byte — and the card
  says nothing, bracketed by a reader either side that shows it present and answering throughout
  (BENCH-RULE 2b). So the silence is a refusal, not an absence, and the acceptance above is the card
  MATCHING the address rather than ignoring the flag (BENCH-RULE 2).
- **THE RE-ADDRESS SEAM WORKS, in miniature.** The restore is addressed to the UID the previous
  write PRODUCED, and it is accepted. That is exactly what the app does between block 56 and block
  57: write 56, take the new address, address the next frame to it. The one thing addressing this
  sequence costs is the one thing this line exercises.

**SO AN ADDRESSED WRITE TO BLOCK 56 IS NOW MEASURED ON ALL THREE GEN1 CHIPS.** ST LRi2K through the
wipe's sweep, NXP ICODE SLIX through the clone's conversion path, and NXP ICODE SLIX-S here. That was
the last gap in the premise `b312653` rests on, and SLIX-S was the one holding it open: its 40-block
claim puts the sweep's reach at 47, so nothing had ever addressed a frame to that register on this
silicon.

**Restored** (BENCH-RULE 9). The card is back on `E0 04 01 10 A1 A2 A3 A4`, confirmed by the last
reader in the transcript, and it wore that before the run rather than a factory UID.

### The Flipper — all three gen1 cards, 2026-09-26

`Write UID` -> target `E0 11 22 33 44 55 66 77` -> "Not gen2 magic card" -> gen1 opt-in accepted.

| card | chip | screen | UID after | DSFID after |
|---|---|---|---|---|
| `SL2S5302` | NXP ICODE SLIX-S | **plain Success** | `E0 11 22 33 44 55 66 77` | `00` |
| `lri2k-keychain` | ST LRi2K | **plain Success** | `E0 11 22 33 44 55 66 77` | `02` |
| `slix-1k-50x28` | NXP ICODE SLIX | **plain Success** | `E0 11 22 33 44 55 66 77` | `00` |

**THE WHOLE TARGET, ON ALL THREE.** Not the tail, which is the failure this target was chosen to
expose: block 56 carries the printed tail and 57 the printed head, so `E0 11 22 33 44 55 66 77`
against each card's own UID differs in BOTH HALVES and a run that wrote 56 and lost 57
would have left the head behind. None did. The re-address between the two halves holds on every gen1
chip here.

**Plain Success, not Partial.** A Write-UID has no payload to follow, so a verified UID is a clean
success — that is `0fd69e8`'s outcome fix arriving on hardware, on the path it was written for.

**Two incidental controls, neither planned.**

- **DSFID survived, per card**: `02` on the LRi2K against `00` on the other two, which is what each
  wore going in. A Write-UID sends the four backdoor frames and nothing else, and if the sequence
  had scribbled outside the UID registers this is where it would show. It did not.
- **The type line now reads `Emosyn-EM Microelectronics USA` on all three**, because `uid[1]` is
  `0x11` in the target. Three different chips, one type line, decoded from the costume. That is the
  release-notes correction this round already made, demonstrated live -- and it is the same mistake
  that produced `gen-2-card`'s "EM-Marin" label, which the round also had to take back out.

## ⚠️ What the run left behind, and what each card must go back to

`E0 11 22 33 44 55 66 77`, on `SL2S5302`, `lri2k-keychain` and `slix-1k-50x28` at once. They are
indistinguishable to an inventory while that holds, and the SDK's is 1-slot, so with two of them near
the antenna anything addressed can reach either. **Restore them one at a time, and identify each by
its tape rather than by what it answers.**

The values to restore to, from the de-arm probe's own record of the states it found:

    lri2k-keychain    E0 02 22 24 50 00 83 03   (its factory UID; it was restored to this then)
    slix-1k-50x28     E0 04 01 10 5E ED 00 01
    SL2S5302          E0 04 01 10 A1 A2 A3 A4

`SL2S5302`'s is confirmed current -- it read exactly that at the head of today's pm3 run. The other
two were NOT re-read before today's Flipper run, so those two lines are the last recorded value
rather than an observed one.

### RESTORED 2026-09-26, to the INVENTORIED ORIGINALS rather than to those

mfcarroll's call, and it puts two of the three somewhere they have not been for weeks.

    SL2S5302        -b 56 F8350003  -b 57 500204E0  -> E0 04 02 50 03 00 35 F8   ( ok, ok )
    lri2k-keychain  -b 56 03830050  -b 57 242202E0  -> E0 02 22 24 50 00 83 03   ( ok, ok )
    slix-1k-50x28   -b 56 8C062620  -b 57 500104E0  -> E0 04 01 50 20 26 06 8C   ( ok, ok )

Each read back byte-identical to `original.uid` in `tools/tag-inventory.json`. Six unaddressed
backdoor writes, six `( ok )`, three cards, generated by `tools/gen1-addressed-frames.py --restore`.

**AND THE TYPE LINE PROVED ITSELF BOTH WAYS, IN ONE SITTING.** Wearing the target UID, all three
cards printed `Emosyn-EM Microelectronics USA`. Wearing their own, the same three printed:

    SL2S5302        NXP (Philips); ICS5302/ICS5402 ( SLIX-S )
    lri2k-keychain  ST Microelectronics SA France
    slix-1k-50x28   NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )

Three correct manufacturers, three correct chips, from the same readers on the same cards minutes
apart. Nothing about the silicon changed between the two sets; only eight bytes of UID did. That is
the release-notes correction this round makes -- a reader's type line describes the UID the card
wears, not the chip underneath -- demonstrated in both directions rather than argued. It is also
exactly how `gen-2-card` came to be labelled an EM-Marin for months.

**State change worth knowing:** `SL2S5302` and `slix-1k-50x28` are no longer wearing the `slix_28`
fixture costume, so they now identify correctly from a bare `hf 15 reader`. `slix-1k-coin18` still
wears it.
