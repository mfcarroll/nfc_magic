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

---

# RESULTS — both tests pass, and the wording did not

Run by mfcarroll on the FAP installed from `cab704d`. Predictions above were committed first.

## Test A — PASSED

Wipe: **64/64**, exactly control 1's figure, so the card was in the state the clone assumed.
Clone `iso15693_slix_28`: the note fired, holds **64**, reports **28**.

Both halves matter. The note firing at all is the guard not suppressing a real finding. The 64 is the
survey's own read-derived figure -- the card advertises 28 by then, so it cannot have been read off
it -- and it agrees with the physical top the inventory established independently.

## Test B — PASSED

`SL2S5302` before, and it is NOT wearing the file's UID, so this is the gen1 path and not a
conversion:

    UID....... E0 04 01 10 F1 F2 F3 F4
    SYSINFO... 00 0F F4 F3 F2 F1 10 01 04 E0 02 00 27 03 02
    DSFID 0x02   AFI 0x00   IC ref 0x02   40 blocks

Clone `iso15693_slix_28`, gen1 opt-in:

    Clone finished
    All data written.
    Card still reports
    40 blocks, IC ref 02.

**No size note** -- which is the result. Blocks 28-39 are readable and above the SOURCE, and the card
reports 40, so they are not above its CLAIM. A guard comparing against the source count would have
printed "larger than it claims" here about a card that is exactly the size it says.

After, and verified against the file the device actually opened -- `iso15693_slix_28.nfc` was pushed
and read back byte-identical before the session:

    UID....... E0 04 01 10 A1 A2 A3 A4     the source's
    DSFID..... 0x00                        moved 02 -> 00, the source's
    IC ref.... 0x02                        unchanged, which is the note's whole subject
    40 blocks                              unchanged, same reason

`hf 15 dump`: blocks 0 and 1 hold `A5 00 5A 00` and `A5 01 5A 01`, blocks 2-39 zero. The source's
Data Content is `A5 00 5A 00 A5 01 5A 01` then zeros through block 27, so the copy is byte-exact and
28-39 are the card's own empty tail. That empty tail is why the configuration note reached the
summary line rather than the residue note -- the second of the two predicted forms.

## What the runs changed

The size note's wording, which fired correctly and read backwards: it led with the physical top,
which the user has not been told about yet, before the count the card actually reports. It now says
what the card reports first, and both figures as counts rather than a count against a block index.

## State left behind

`gen-2-card`: claiming 28 over 64 physical, clean tail, wearing `E0 04 01 10 A1 A2 A3 A4`. Restore by
cloning any 64-block source onto it.
`SL2S5302`: wearing `E0 04 01 10 A1 A2 A3 A4` with its own 40-block geometry and IC ref 02 intact.
**Both now wear the same UID**, which is the state that sends a later re-clone of that file down the
conversion path -- deliberate on neither card, so move one before testing anything else with it.

---

# Test C — the reworded size note. Predicted before the run

FAP installed from `923ca1a`. Covers both wording commits in one run.

`gen-2-card` is already in the state this needs, left by test A: claims 28, holds 64, tail clean, and
wearing `E0 04 01 10 A1 A2 A3 A4`. **No wipe this time** -- blocks 28-63 were zeroed by test A's wipe
and nothing has written them since, so the tail is still clean and the size note keeps the summary
line rather than being outranked by residue.

**Clone `iso15693_slix_28` onto it again.** The card wears the file's UID already, so the gen2 verify
passes on the first look; it is gen2 magic, so the CFG frame lands as before. The source stops at
block 28, so nothing is written near 56/57 and the run does not convert.

**PREDICTED summary** -- the two numbers in the other order from the last run, both as counts:

    Clone finished
    All data written.
    Card reports 28 blocks,
    but holds 64.

**PREDICTED Details**, one bullet, now naming the file:

    - The card reports 28 blocks, the same as the file, but holds 64, and still answers
      individual reads to those higher blocks. Some readers may detect this.

**It can fail three ways, and they are distinguishable.** The numbers the wrong way round -- "reports
64 blocks, but holds 28" -- is the printf-argument slip. A summary reading "holds 64 blocks, reports
28" is a stale FAP rather than a defect. And Details missing "the same as the file" means the clause
is gated on something other than the two counts, since here they are both 28.

## The other branch of that clause is not reachable here

Details drops "the same as the file" where the card's reported count and the file's differ. Producing
that needs a card that answers above its own claim AND whose claim is not the file's count -- and the
gen2 CFG frame sets the claim to the file's count every time, so no gen2 clone can reach it. gen1
cards keep their own claim, but every gen1 card here stops exactly at it.

`slix2-gold-30mm` is the one card on the shelf that answers past its claim (79 advertised, 82
physical). It is also unclassified -- no write probe has ever been run on it -- so reaching the survey
means offering it the gen1 opt-in, which writes four real data blocks on a tag that may not be magic.
**Not worth it for a display string**: the branch is pinned by a test that asserts the clause's
absence, and a mutant that always emits it dies.

---

# Test D — the branch I said was unreachable. mfcarroll was right

**Correction.** I said the file clause's other branch could not be reached without offering the gen1
opt-in. That was wrong, and the poller says so at the branch itself: the gen2 verify proves *the UID
is now the target*, not *the card is magic*. **A file carrying the card's OWN UID passes it without
anything having happened** -- so the run goes straight to the data pass and the survey on a tag that
is not magic, with no backdoor write that lands and no opt-in ever offered.

`tools/test_nfc/iso15693_slix2_selfuid_8.nfc` is that file, pushed and read back byte-identical.
UID `E0 48 03 00 01 CD F1 36`, 8 blocks, IC ref 01, AFI 00, DSFID 00 -- the card's own values for
everything except the count, so the identity write is a no-op and the block count is the only thing
left differing.

## STOP if the UID does not match

`hf 15 info` first. If the card is not wearing `E0 48 03 00 01 CD F1 36`, the gen2 verify FAILS and
the app offers the gen1 opt-in -- which on this card overwrites four blocks of real memory, because
56/57/62/63 are in range here rather than above capacity as they are on the 28-block tags. **Decline
it if it ever appears**: seeing that screen means the file is wrong, not that the test is proceeding.

Take a full `hf 15 dump` baseline as well. This card has never been written to.

## What is actually being learned

Two things, and the second is worth more than the first.

1. The size note must NOT say "the same as the file" here. Card reports 79, file says 8.
2. **The clone survey has never met a card with no read edge.** This one answers a read at every
   address in the 8-bit block space, measured 2026-09-08, so the survey's absent-run stop can never
   trip and only the ceiling or the pass budget ends it. Every survey bench so far has been on a card
   whose reads stop somewhere.

## PREDICTED

- **No gen1 opt-in.** The verify passes on the first look.
- "Clone finished", and "All data written." -- 8 blocks onto a card that takes writes.
- Details carries THREE bullets.
- The size note reads **"The card reports 79 blocks but holds N"** with **no "the same as the file"**.
  That clause's absence is the test.
- The configuration note names the counts only -- file 8 against card 79 -- and not the IC ref, since
  both are 01.
- **N is 256 if the survey reaches the ceiling, lower if the ten-second budget cuts it first.** Either
  is a result and neither is a failure; which one it is, is the new information.

**NOT predicted, deliberately:** the residue range. Nothing here knows what this card returns above
its real memory -- zeros, a mirror of the low blocks, or something else -- and guessing it would turn
whatever comes back into a confirmation.

Expect the write to take noticeably longer than the other runs: up to ~248 extra reads.

## Judge the wording on the screen it produces

If N comes back 256, the note will tell the user the card "holds 256 blocks". By this app's own
evidence that is what the card says -- a block that answers a read exists is the discriminator the
whole survey rests on -- but this chip is the one that breaks the premise, and "holds" may be a
stronger word than reads alone can support. Worth reading on the real screen before deciding whether
it needs changing.

## Restore

Blocks 0-7 only. Block 21's data is below neither and is not touched. Restore from the baseline dump.
