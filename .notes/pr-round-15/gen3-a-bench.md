# gen3-a — what a real gen3 card does under the gen1/gen2 functions this PR ships

**Scope.** The PR ships gen1 and gen2 write paths and says two things about gen3: a wipe can brick
one, and "A gen3 card ignores the gen2 backdoor". The first is attributed to @0x6r1an0y and stays
attributed. The second has never been put to a gen3 card — every gen3 sticker in the shipment is
untouched. This run tests only where gen3 meets the functions we ship, on ONE card. It is not gen3
support and does not become any.

**One card.** Only `gen3-a` is opened; `gen3-b..e` stay in the packet (BENCH-RULES 7). If gen3-a
bricks despite the guards, four identical cards remain to learn from.

## What we already know (step 1), and the three things never to do

From `.notes/gen3-candidate-slix2-gold.md` and proxmark's `cmdhf15.c` (the V3 support 0x6r1an0y
wrote), confirmed on `slix2-gold-30mm`:

- **`0x10`/`0x11` (blocks 16/17) is the UID register.** Writing 16 moves the UID to the value the
  mapping implies, reversibly — this is `csetuid`, "the repeatable half". Restore = write the
  original 16/17 back.
- **`0x14`/`0x15` (blocks 20/21) is the configuration signature.** Zeroing these on an un-finalized
  card **bricks it permanently** — proxmark guards `cfinalize` on the signature for exactly this
  reason. Whether gen3-a is finalized is unknown, so treat it as brickable.
- **`cfinalize` is irreversible**: it erases the configuration area and locks the UID forever.

**NEVER, on any gen3 card:**

1. **Run the app's Wipe.** It sweeps every block, 20/21 included, in its first two dozen writes; and
   because a gen3 card answers a write at every address, the absent-run stop never trips, so it runs
   to the ceiling. There is no safe wipe here.
2. **Write blocks 20/21** (`0x14`/`0x15`) by any route.
3. **Send finalize** (`hf 15 cfinalize`, or anything resembling it).

Everything below stays clear of all three: it writes only 16 (reversibly) and the gen1 data blocks
56/57/62/63, and reads 20/21 to confirm they never moved.

## The addition your plan needs: a full baseline FIRST (folded into step 2)

`slix2-gold-30mm` had a committed full sweep to restore from. **These stickers do not.** So before
any write, capture the whole addressable space and commit it — otherwise a restore value is gone the
moment it is overwritten (BENCH-RULES: compare against the artifact, and rule 9). Read-only:

    hf 15 info                     -> record UID, DSFID, AFI, IC ref, block count/size
    hf 15 reader                   -> the live UID, for bracketing
    hf 15 dump                     -> the full block read; SAVE the transcript
    hf 15 rdbl -b 16 ; -b 17       -> the UID register, called out so the restore value is explicit
    hf 15 rdbl -b 20 ; -b 21       -> the config signature, so "unchanged" can be checked later

The app's **Info** scene is read-only and safe to open too; note what it shows against `hf 15 info`.
Do not run the app's scanner past Info into any write path yet.

## Steps 3–5, each a prediction before the run (BENCH-RULES 5), each bracketed (2b)

Bracket every write with `hf 15 reader` immediately before and after, so a silence is a refusal and
not an absent card.

**Step 3 — the UID register, at the bench, reversible.** This is a proxmark write, not an app one:
the app cannot move a gen3 UID (its Write UID tries gen2, which gen3 ignores). Use the one-frame
check already recorded for the gold tag — write 16 to a marker value, confirm the UID tail follows,
write the captured original back:

    (original 16 recorded above)
    hf 15 wrbl --ua -b 16 -d AABBCCDD   ; hf 15 reader   -> UID tail becomes DD CC BB AA?
    hf 15 wrbl --ua -b 16 -d <original> ; hf 15 reader   -> back to the baseline UID

- **Predict:** the tail moves to `DD CC BB AA` and returns. That is the gen3 UID mechanism, same as
  the gold tag.
- **If it does NOT move:** gen3-a is not the gold tag's mechanism — record it and STOP the UID work;
  it is a different variant.

**Step 4 — the gen2 sequence, via the app, expect no change.** The app's **Write UID** begins with
the gen2 backdoor. Set a target differing from gen3-a's own UID and start it.

- **Predict:** gen2 leaves the UID unchanged (this is the release note's claim, first tested here),
  so the app reports "Not gen2 magic card" and offers the gen1 opt-in. Nothing at 20/21 moves — the
  gen2 backdoor writes its own register space via `0xE0`/`0x09`, not the config blocks. **Decline the
  opt-in for now**, read 16/17 and 20/21, confirm both unchanged.
- **If the UID DID change on the gen2 step:** stop — that contradicts the release note, and the note
  is what needs rewriting, not the card. Record `hf 15 reader` before/after.

**Step 5 — the gen1 sequence, via the app, expect only ordinary data-block changes.** Re-run Write
UID (or a clone) and this time **accept** the gen1 opt-in. gen3-a is large enough that 56/57/62/63
are ordinary data blocks, so the gen1 sequence writes them as ordinary WRITE BLOCKs.

- **Predict:** blocks 56/57/62/63 take the gen1 sequence's bytes (56 the UID tail, 57 the head, 62
  `00…`, 63 `69 96 00 00`); the UID at 16/17 does NOT move; 20/21 do NOT move; the app reports the
  gen1 outcome for a non-gen1 card (the UID it wrote does not read back, so Fail / gen1 didn't take).
  Only the four data blocks changed, all recoverable from the baseline.
- **If 16/17 or 20/21 moved:** stop and record — the gen1 path reached a register it must not.

## Step 6 — restore, and record it (BENCH-RULES 9), never via Wipe

Write the baseline values back to any block that changed — 16/17 if step 3's restore was skipped, and
56/57/62/63 from step 5 — with ordinary `hf 15 wrbl` frames (`tools/gen1-addressed-frames.py` can
build the set). Then `hf 15 dump` and diff against the baseline transcript; it must be byte-identical.
The app's Wipe is never the restore tool here.

## What this settles, and what changes if it holds

- **Confirms** gen3-a carries the gen3 UID mechanism (step 3) and that the gen2 backdoor does not move
  its UID (step 4) — which is the release note's claim, currently untested. If step 4 holds, the note
  "A gen3 card ignores the gen2 backdoor" becomes measured rather than inferred, and the reply/notes
  can say so, scoped to the one card.
- **Leaves** the brick claim exactly as attributed: nothing here finalizes or touches 20/21, so it
  tests none of it.
- If it all holds and only the wording moves from inferred to measured, that is one self-contained
  commit on the end (or a fold into 08, where the gen3 note lives, if churn allows — decide then).

# RESULTS

## Baseline (step 2), in progress — 2026-09-29

Initial read, `hf 15 info`:

    UID....... E0 48 03 00 12 85 5F 76
    SYSINFO... 00 0F 76 5F 85 12 00 03 48 E0 00 00 4F 03 01
    DSFID..... 0x00      AFI....... 0x00      IC ref.... 0x01
    memory.... 4 (or 3) bytes/block x 80 blocks, 320 total

`E0 48 03` is the same manufacturer/family prefix the gold tag reads (`E0 48 03 00 ...`). It
advertises **80 blocks** — a setting, not necessarily its capacity, so the full sweep below is the
test of what actually answers past 80.

## PASSED, 2026-09-29 — every step as predicted, nothing near the brick blocks

- **Step 3, the UID register at the bench, reversible both halves.** Wrote block 16 `AABBCCDD` ->
  UID `...DD CC BB AA` (the tail, byte-reversed); restored. Wrote block 17 `11223344` -> UID
  `44 33 22 11 ...` (the head); restored. So 16/17 DRIVE the UID, not merely hold a copy -- the
  gold-tag increment, now measured on a card whose signature is documented.
- **Step 4, the app's gen2 backdoor: UID unchanged.** Write UID to a target; `hf 15 info` before
  and after both `E0 48 03 00 12 85 5F 76`, so the app reported "Not gen2 magic card" and offered
  the gen1 opt-in (declined). 16/17, 20/21 and 56/57/62/63 all unchanged -- the gen2 sequence
  reached nothing. **First time "a gen3 card ignores the gen2 backdoor" has been put to a gen3 card.**
- **Step 5, the app's gen1 opt-in accepted.** Screen: "gen1 failed / UID didn't take, so not a gen1
  card. 56/57/62/63 may be overwritten." After: 56 `FF 5F 85 12`, 57 `00 03 48 E0`, 62 `00 00 00 00`,
  63 `69 96 00 00` -- the gen1 sequence written as ORDINARY DATA (target = the card's own UID with
  the last byte 76->FF, so the write was attempted). The UID did NOT move (16/17 unchanged), and
  **0x14/0x15 were untouched** (`A5 2B 44 2C` / `21 AE 93 00`, as at baseline). So on a gen3 card the
  gen1 path costs exactly those four data blocks, moves no identity, and reaches no brick block.
- **Step 6, restore.** 56/57/62/63 back to zero; UID `E0 48 03 00 12 85 5F 76`; full dump 0-79
  byte-identical to the baseline (only 16/17, 20/21 non-zero). Card left un-finalized, config intact.

**What it settles for the PR.** The gen3 bullet's mechanism -- gen2 ignored -> lands on the opt-in ->
accepting writes four ordinary blocks, and that is the whole cost -- is now MEASURED on a genuine
gen3 card, not reasoned. The brick claim is untouched by this run (no wipe, no write to 0x14/0x15, no
finalize), so it stays exactly as attributed. Only the app's Wipe reaches the brick blocks, which is
why the release notes single it out.

---

Full sweep (reads only), `tools/baselines/gen3-a_2026-09-29_full-sweep-0-255.txt`, 256 blocks:

- **Every address 0–255 answers a read**, contiguous, no gaps. So 80 is a setting, not a capacity.
- **128 cells mirrored across the 8-bit space**: block `0xNN` reads identical to `0xNN+0x80` at all
  four registers (16≡144, 17≡145, 20≡148, 21≡149). Same shape as `slix2-gold-30mm`.
- **UID register at 0x10/0x11, confirmed by READ alone.** Block 16 = `76 5F 85 12`, block 17 =
  `00 03 48 E0` — the UID `E0 48 03 00 12 85 5F 76` in the gen3 mapping, each half reversed (tail
  `12 85 5F 76` → `76 5F 85 12`, head `E0 48 03 00` → `00 03 48 E0`). This is the mechanism; the
  read shows the register HOLDS the UID, a write (step 3) would show it DRIVES it.
- **Config signature at 0x14/0x15 = an EXACT match to V3 config mode**: block 20 = `A5 2B 44 2C`,
  block 21 = `21 AE 93 00`, against config-mode `A5 2B 44 2C` / `21 AE 93 00`. Not the gold tag's
  unrecognised pair, and not the finalized pair.
- Non-zero blocks: only 16/17, 20/21, and their mirrors 144/145, 148/149. Everything else zero. No
  lock bits set anywhere.

**What the baseline settles, and how it sharpens the guards.** gen3-a is a genuine V3 card in
**config mode — un-finalized** — the first the project has placed by its documented signature. Two
consequences, both making the guards stricter than for the gold tag:

- the brick risk is no longer "might be un-finalized": it IS the un-finalized state the attributed
  report is about, so zeroing 0x14/0x15 would brick it, not maybe.
- proxmark's `hf15_magic_v3_is_config_mode()` returns TRUE here (both signature blocks match), so
  `cfinalize` would be ACCEPTED on this card — the guard that refused the gold tag does not protect
  it. Never send it.

Step 3's UID write stays reversible (config mode = the repeatable half), and the restore value is
captured: block 16 = `76 5F 85 12`, block 17 = `00 03 48 E0`.
