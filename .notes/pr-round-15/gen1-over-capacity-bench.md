# A gen1 clone of a file larger than the card — predictions, written before the run

**Why this run.** The 2.3 release notes now say an over-capacity clone "is a **Success** carrying a
note that the file is larger than the card — on gen2 the card then advertises more blocks than it
physically holds", and the code sends every gen1 case to "Clone finished" rather than the
over-capacity screen: gen1 has no configuration register, so the card goes on reporting its own
count and the survey's geometry note fires beside the empty tail. A host test pins that routing
(`test_over_capacity_with_a_survey_note_goes_to_clone_complete`). Nothing has run it on hardware:
`known-count-guard-bench.md` Test B was the reverse, a smaller file onto a larger gen1 card. This is
the only user-visible path the whole-PR review's fold described anew without a bench behind it.

## Setup

- **FAP**: the build benched 2026-09-29 (success tone on CloneComplete). Anything older lacks the fold.
- **Source**: `iso15693_lri2k_56.nfc` -- 56 blocks x 4, UID `E0 02 08 B1 B2 B3 B4 B5`, DSFID 00,
  AFI 00, IC ref `1A`, blocks 0-1 non-zero and 2-55 zero. It stops at 55, so 56/57/62/63 are not in
  it. **Confirm the copy on the device is the repo's**: pull `/ext/nfc/iso15693_lri2k_56.nfc` back and
  diff it against `tools/test_nfc/`, or push it fresh (BENCH-RULES, first rule).
- **Card**: `slix-1k-50x28` -- NXP ICODE SLIX, gen1, 28 blocks, IC ref 01. Inventoried as UID
  `E0 04 01 50 20 26 06 8C`, DSFID 00, AFI 00. **Read it first and record what it holds now** -- UID,
  DSFID, AFI, blocks -- rather than trusting the inventory.

## Run

Magic -> ISO15693 -> clone `iso15693_lri2k_56` onto `slix-1k-50x28`. Sound on.

## Predictions

1. **The gen1 opt-in appears** ("Not gen2 magic card"): gen2 leaves the UID unchanged. Accept it.
2. **"Clone finished"**, with the **success tone**. Body, three lines:

       All data written.
       Card still reports
       28 blocks, IC ref 01.

   Both geometry halves moved: the file asks for 56 blocks and IC ref 1A, the card is 28 and 01.
3. **Buttons**: Finish on the left, Details on the right.
4. **Details -> "Clone notes"**: a line opening `- Didn't fit on the card:` listing the empty blocks
   from 28 up (possibly cut short with "..."), and the configuration note naming both sides -- what
   the file asked for, what the card reports.
5. **NOT** any of: Partial; "Card too small"; the over-capacity screen ("Holds ..."); the gen1
   "56/57/62/63" caveat, since the file never reaches those blocks.
6. **The card afterwards**: UID `E0 02 08 B1 B2 B3 B4 B5`, blocks 0-1 the file's pattern and 2-27
   zero, still 28 blocks and IC ref 01 -- gen1 has no configuration register to program.
7. **Time**: a second or two longer than a 28-block clone, for 28 refused writes and their reads.

## If a prediction fails

- **The over-capacity screen instead of "Clone finished"**: the geometry note did not fire, so this
  card reported 56 blocks after the clone -- a gen1 card with a programmable count, which the app
  does not expect. Record `hf 15 info` before and after.
- **Partial or "Card too small"**: the tail was classified as lost data, not as blocks past capacity
  -- the read-probe found blocks 28+ answering. Record a read of block 28.
- **Error tone on "Clone finished"**: the FAP predates the fold.

## Restore, and record it (BENCH-RULES 9)

1. **Wipe** the card with the app, to clear blocks 0-1. Safe on this card: its claim is 28, so the
   sweep stops at block 35 by the reach rule, short of the UID registers -- the UID must not move,
   and the wipe's re-read will say so.
2. **Write UID** back to the UID recorded in Setup, accepting the gen1 opt-in.
3. Read it back and note the result below.

# RESULTS

**PASSED, 2026-09-29, mfcarroll on `slix-1k-50x28`.** Every prediction held:

- the gen1 opt-in appeared, accepted;
- **"Clone finished"** with the **success tone**, body "All data written. / Card still reports / 28
  blocks, IC ref 01.";
- Finish left, Details right;
- Details "Clone notes": the leftover blocks under "Didn't fit on the card", and the configuration
  note naming both sides;
- none of the excluded screens (no Partial, no "Card too small", no over-capacity "Holds ...", no
  56/57/62/63 caveat);
- restored: wiped (UID did not move, as the reach rule predicts for a claim of 28), Write UID back
  to `E0 04 01 50 20 26 06 8C` via the gen1 opt-in, read back byte-identical.

So the release-note wording is confirmed on hardware: a gen1 clone of a file larger than the card is
a Success that says the file is larger than the card, and the card goes on reporting its own count.
Nothing in sync points 06/08/09 about this path needs changing.
