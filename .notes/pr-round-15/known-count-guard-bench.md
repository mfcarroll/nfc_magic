# The known-count guard — predictions, written before the run

FAP installed 2026-09-26 from `cab704d` (`fbt launch`, not just built). `iso15693_poller.c`
recompiled from source with no warnings. All nine test sources pushed to `/ext/nfc/`, and
`iso15693_slix_28.nfc` read back off the device and diffed byte-identical.

## What changed, and what the bench can and cannot reach

`clone_holds_more` -- the "card is larger than it claims" note -- now requires the card to have
STATED a block count. Before, an unknown count read as zero and the first readable block above the
source tripped the note.

**The new branch itself is NOT reachable on this bench, and no run here should claim it was.** It
needs a card that answers GET SYSTEM INFO without the MEMORY flag, or a GET SYSTEM INFO that fails.
Every card in the inventory advertises all four flags (0x0F, checked in `tools/tag-inventory.json`),
so the first is impossible here. The second sits in the gap between the last data-block write and the
survey: lifting the card there also kills the survey's own reads, so the run reports nothing either
way and the two outcomes are indistinguishable -- a control that cannot fail (rule 2). It is covered
by two harness tests and four killed mutants, and that is where it stays.

**What the bench IS for: proving the guard did not break the two paths that do run.** It sits
directly in the line that decides the size note, so both directions need a card behind them.

## State at the start of the session — read on the proxmark, not assumed

`gen-2-card`: **advertises 64, and 64 is its real physical top** (inventory; `hf 15 info` reports
only the claim, so the capacity figure is the inventory's read-derived one, not this transcript's).

    UID....... E0 04 01 10 5E ED 00 01
    SYSINFO... 00 0F 01 00 ED 5E 10 01 04 E0 00 00 3F 03 0F
    IC ref.... 0x0F        4 bytes/block x 64 blocks

Claim and capacity agree, so the card has nothing to say about its own size right now -- which is
what Test A needs, because the clone is what creates the disagreement.

Note what the predicted screen then means: by the time the note is printed the card ADVERTISES 28,
so the 64 in "Card holds 64 blocks" cannot have been read off it. It is the survey's own figure,
arrived at by reading upward until blocks stop answering. Test A therefore also checks that figure
against a physical top known independently.

## The wipe is a prerequisite control, not just a setup step

Test A's first step is a wipe, and it is worth predicting rather than skipping past: it should report
**"Wipe complete / Cleared 64 blocks. Card claims 64."** That is control 1's recorded result for this
card, unchanged. If it reports anything else the card is not in the state Test A assumes and the
clone should not be run yet.

## Before anything else, check the other card

Card state moved since the last session and must not be assumed.

`gen-2-card` is read above and is ready. The one still to check:
- **`SL2S5302`'s UID.** If it already wears `E0 04 01 10 A1 A2 A3 A4` -- `iso15693_slix_28`'s UID --
  then a re-clone takes the gen2 path and converts, which is a different test and muddies B. Clone
  something else onto it first, or wipe it, to move the UID off.

## Test A — the note must still FIRE (`gen-2-card`, gen2 magic)

1. **Wipe** with the app. Clears all 64, so the tail above the source is empty.
2. **Clone `iso15693_slix_28`** (28 blocks, IC ref 03).

The gen2 CFG frame writes the count down to 28 and the IC ref to 03, so the card reports the
source's geometry while still answering reads to 63. Empty tail, so no residue and no configuration
note -- the size note is alone, which is what makes this the discriminating run.

**PREDICTED:**

    Clone finished
    All data written.
    Card holds 64 blocks,
    reports 28.

Details -> "Clone notes", exactly one bullet: *The card is larger than it claims. It reports 28
blocks but answers reads up to block 63. Some readers may detect this.*

**It can fail, and here is how.** If the guard never marks the count known, `holds_more` never fires;
with no residue and matching geometry there is nothing to report at all, and the app shows the plain
**Success popup** instead of a result screen. Popup instead of "Clone finished" IS the failure.

## Test B — the note must NOT fire on an honest card (`SL2S5302`, NXP SLIX-S, gen1)

Clone `iso15693_slix_28` with the gen1 opt-in. gen1 has no CFG register, so the card goes on
reporting 40 blocks and IC ref 02 truthfully. Blocks 28-39 are readable and above the SOURCE, but not
above the CARD'S CLAIM -- so the size note must stay silent.

**PREDICTED**, if 28-39 hold data:

    Clone finished
    All data written.
    Blocks 28-39 hold
    older data.

or, if that stretch is empty:

    Clone finished
    All data written.
    Card still reports
    40 blocks, IC ref 02.

Details: the residue note if it applied, plus the configuration note naming both sides (file 28
blocks / IC ref 03 against card 40 / IC ref 02). **No size note in either case.**

**It can fail the other way.** A guard comparing against the SOURCE count instead of the card's claim
prints "larger than it claims" here about a card that is exactly the size it says. Seeing that line
on this card is the failure.

## Nothing else needs re-running

The fold changed one behavioural line. Everything else it carried was comment, release-notes or fork
prose, so the round's earlier bench results stand as recorded.

## Record what the runs leave behind

Test A leaves `gen-2-card` claiming 28 over 64 physical with a clean tail, wearing
`E0 04 01 10 A1 A2 A3 A4`. Test B leaves `SL2S5302` wearing the same UID with its own 40-block
geometry intact -- which is the state that makes a later re-clone take the conversion path.
