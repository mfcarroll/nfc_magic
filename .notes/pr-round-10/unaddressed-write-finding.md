# TI Tag-it HF-I Plus and the WRITE BLOCK flags

> **CORRECTED 2026-09-24. The conclusion below is wrong, and the measurement under it is sound.**
> What TI wants is the **OPTION flag**, not an address. All four combinations, on `white-coin`:
>
> | flags | addressed | option | result |
> |---|---|---|---|
> | `0x02` | no | no | refused, error 0x01 |
> | `0x22` | yes | no | refused, error 0x03 |
> | `0x42` | no | **yes** | **accepted** |
> | `0x62` | yes | **yes** | **accepted** |
>
> The OPTION flag is necessary and sufficient; the addressing is orthogonal. The reading below took
> the 0x01 answer to `0x02` as a refusal of the UNADDRESSED form, having changed only that variable
> -- and `hf 15 wrbl`, the control, silently forces OPTION on for TI silicon, so the frame that
> "worked addressed" differed in two bits. Both halves of the comparison moved.
>
> The app still could not write this card, that is still a merge blocker, and it is fixed -- by the
> OPTION flag. **Addressing is the safety improvement #251 filed it as, not a compatibility
> requirement.** Everything in this file that says otherwise is superseded by that.


## The measurement

On `white-coin` (TI Tag-it HF-I Plus, IC ref 0x8B, 64x4), same tag, same block, three commands:

```
hf 15 wrbl      -b 8 -d 11223344   ->  ( ok )
hf 15 wrbl --ua -b 8 -d 55667788   ->  error 1: "The command is not supported"   ( fail )
hf 15 wrbl      -b 8 -d 55667788   ->  ( ok )
```

Addressed works, unaddressed is refused with ISO15693 error **0x01**, addressed works again. The
third command is the control: the tag is not damaged and not write-protected.

## Why this breaks the app

**Every ISO15693 write this app sends is unaddressed.** That is the whole of #251 — the SDK's
`iso15693_3_poller_write_block` builds `SUBCARRIER_1 | DATA_RATE_HI` (0x02) with no UID, and the
app's own backdoor frames use `ISO15693_MAGIC_FLAGS`, also 0x02.

So on this silicon the app cannot write a data block at all. A wipe clears nothing and reports
"Wipe failed / No blocks could be cleared". A clone sets the UID through the gen2 backdoor and then
fails every data block.

## It is NOT a regression

The app's write path is unchanged since the 2026-08-17 regression five passed — checked with
`git diff` over `iso15693_poller.c` from 2026-08-16 to HEAD: the only non-comment changes are
`COUNT_OF`, renamed constants, the mark/unmark helpers, a moved forward declaration and
`block_is_empty`. Nothing that reaches the wire.

What differs is the CARD. The August regression ran on the original gen2 sample — **EM-Marin, IC
ref 0x0F, 66 advertised / 64 physical** — which accepts unaddressed writes. `white-coin` and
`black-tag` are **TI Tag-it HF-I Plus, IC ref 0x8B**, and arrived 2026-08-24, a week later. The
notes have carried "re-run the regression five on one of them; that is the outstanding piece" ever
since. This is that run, and it found the reason it mattered.

## What it does to #251

#251 was filed as a BYSTANDER hazard: unaddressed frames reach any tag in the field, so a second
tag silently takes the write. That still holds. But it is now also a **COMPATIBILITY LIMIT** —
some silicon refuses unaddressed WRITE BLOCK outright. Addressing the writes stops being only a
safety improvement and becomes a functional requirement for TI Tag-it cards.

Note the asymmetry that makes this easy to miss: TI accepts the gen2 BACKDOOR (custom 0xE0 frames,
also unaddressed) and refuses the standard WRITE BLOCK. So the card classifies as gen2 magic, the
UID write succeeds, and only the data pass fails.

## Still unknown — the control that has not been run

**Does the EM-Marin card still wipe?** If it does, this is purely a silicon difference and nothing
regressed. If it does not, something else is also wrong and the TI finding is a second problem.
Run that before concluding.

Also unmeasured: whether the four gen1 cards accept unaddressed WRITE BLOCK. They must, since the
gen1 UID sequence is unaddressed and it worked on all five — but that is the backdoor blocks, not
ordinary data blocks. A gen1 CLONE writes data blocks through the same unaddressed path.

## The app's reporting is correct

Worth separating from the defect: the screens behave properly. A wipe that clears nothing reports
"Wipe failed / No blocks could be cleared -- the card accepted no zero-write", which is exactly
what happened, and the clone path has a dedicated screen for "UID written, not one data block
took". The capability is missing; the reporting is not lying about it.

---

# SECOND FINDING — the gen1 caveat fires on sources that do not contain those blocks

Measured 2026-09-13, gen1 clone of a 28-block source onto a 28-block gen1 card:

```
Clone partial
Cloned 28/28 blocks
Not written: 0
gen1: 56/57/62/63 differ
```

Every block took, nothing failed, and the screen still says four blocks differ. On a 28-block
source blocks 56/57/62/63 **are not in the source at all**, so there is nothing for them to differ
from. They also answer no read on this silicon, so "differ" is not even checkable.

**The correct test already exists twenty lines away.** The block-count deduction at
`iso15693_poller.c:618` gates on `iso15693_poller_backdoor_blocks[i] < source_count`, which is why
"Cloned 28/28" is right — nothing was deducted. `test_gen1_small_source_deducts_nothing` pins it.

The caveat does not use that gate. `nfc_magic_scene_iso15693_write_fail.c:270` and
`nfc_magic_scene_iso15693_partial_details.c:153` both fire on `used_gen1` alone, and the second
carries a comment asserting the opposite: "Unconditional: those four blocks differ from the source
whatever the write results above say."

So the same fact is implemented correctly in the poller and incorrectly in both scenes.

**Fixing it is not a comment change.** The scene cannot derive it: on a small source the deduction
is a no-op, so `blocks_total` looks identical either way. The poller has to record whether any
backdoor block was actually skipped, and both scenes gate on that. New result field, two call
sites, and a test alongside the existing deduction one.

Deferred with the addressing work.
