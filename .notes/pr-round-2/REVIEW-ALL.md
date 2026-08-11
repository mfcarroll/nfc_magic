# PR #250 — everything queued for posting

Generated from the files in this directory by `assemble-review.py`. Nothing here has been posted.
Push the fork branch **first** — the replies cite fork SHAs.

| # | goes to | source |
|---|---|---|
| 1 | PR #250 top-level comment | `pr-reply-body.md` |
| 2 | thread on `magic/protocols/iso15693/iso15693_poller.c:897` | `pr-inline/3719022267.md` |
| 3 | thread on `magic/protocols/iso15693/iso15693_poller.c:730` | `pr-inline/3719022269.md` |
| 4 | thread on `magic/protocols/iso15693/iso15693_poller.c:786` | `pr-inline/3719022271.md` |
| 5 | thread on `magic/protocols/iso15693/iso15693_poller.c:657` | `pr-inline/3719022293.md` |
| 6 | thread on `CHANGELOG.md:85` | `pr-inline/3719022325.md` |
| 7 | thread on `magic/protocols/iso15693/iso15693_poller.c:762` | `pr-inline/3719022273.md` |
| 8 | thread on `scenes/nfc_magic_scene_iso15693_write_fail.c:131` | `pr-inline/3719022322.md` |
| 9 | new issue — NFC Magic ISO15693: writes and inventory are unaddressed, … | `issue-1-unaddressed-frames.md` |
| 10 | new issue — NFC Magic: a card removed mid-write is never reported on G… | `issue-2-poller-timeouts.md` |
| 11 | new issue — NFC Magic: Back during a Gen2/Classic write is inescapable… | `issue-3-write-check-back-loop.md` |

---

# 1. Top-level comment on PR #250

Six commits, one per decision: the three blocking items, the two you'd bundle with them, and the
CHANGELOG. Benched on the gen2 card only — no full matrix, as you asked.

I've replied in the threads rather than here for the five items this pass closes, so you can resolve them
as you verify: the read-back power-cycle, the sweep no longer stopping below the advertised count, the
wipe success screen, the wall-clock bound, and the stale CHANGELOG line. Two more threads have replies
despite *not* being fixed this pass, because this round changed what they're worth — the re-probe's
presence-vs-content classification, and the render-branch count.

Your two corrections noted and taken. `BLK_UNLOCK` / `BLK_COMMIT` / "arms the UID change" get marked as
our inference in the documentation batch rather than here, to keep this pass to the hazards.

## Back mid-write — done for ISO15693 only, and I'd like to explain why

This is the one place I've deliberately not done what you approved, so it's here rather than in a thread.

You said yes to swallowing Back for all five magic protocols and that it was yours to call. I've scoped
it to ISO15693, because on the other four it would be a trap rather than a fix.

Swallowing Back is only safe where the write is **guaranteed to report something**. ISO15693 is:
`iso15693_poller_nfc_callback` counts activation failures against
`ISO15693_POLLER_MAX_ACTIVATION_ERRORS` and reports `CardLost` when the budget runs out, and every
write path terminates with a report. The others aren't. Tested rather than argued — same procedure each
time, start a clone and lift the card the moment the popup says "Writing":

| protocol | card | needs a re-activation mid-write? | result |
|---|---|---|---|
| Gen2 / Classic | CUID magic Classic 1K | yes — halts after every block | **no report** (88s observed) |
| USCUID-UL | magic NTAG216 | yes — resets on a failed page | **no report** after >1 min |
| Gen4 | GTU "Ultimate Magic Card" | no — stays activated | reports immediately |
| Gen1A | magic Classic 1K | no — bails on first failure | reports promptly |

A write stalls exactly when it needs a **re-activation** after the card has left. `gen2` halts after
every block, so it needs one to reach the next; once the card is gone that activation fails, the iso3
poller emits `Iso14443_3aPollerEventTypeError`, and `gen2_poller_callback` acts only on `Ready` — so the
state machine is never called again. Confirmed with `log debug`: 88 seconds of `FWT Timeout` at the
100ms retry cadence with not one `GEN2` line in the window. Gen4 discards the same event and is fine,
because it never needs the re-activation.

So on Gen2/Classic and USCUID-UL, swallowing Back would leave the user on a popup that never resolves
and that they can no longer leave — recoverable only by rebooting. I'd rather under-deliver on your
approval than ship that.

Testing it turned up two pre-existing defects on those paths, both filed rather than fixed here:
#252 and #253.

## Bench

Gen2 card, three advertised geometries, which turned out to cover all three cases of the new screen:

| card advertises | physical | result |
|---|---|---|
| 64 | 64 | Success — `Cleared 64 blocks. / Card claims 64.` |
| 70 | 64 | Success — `Cleared 64 blocks. / Card claims 70.` |
| **28** | **64** | **Success — `Cleared 64 blocks. / Card claims 28.`** |

The 28 row is the original residue configuration — the one that produced "36 of 64 blocks survived a
successful wipe". The sweep ignored the claim, ran to the real top at 63, and cleared all 64.

Back mid-wipe now does nothing and still works on the card-search screen. One clone as a smoke test, not
byte-verified. Per-item evidence is in the threads.

Builds clean at Momentum 87.15 and Unleashed 88.2, zero warnings, `clang-format` clean.

## Finding 5's clone half

Closed, per your call. No code change needed — the comments at the `VerifyGen2` match and at the
Write-UID guard already state the position you endorsed.

## Three issues

All filed rather than fixed in passing:

1. #251 — **the unaddressed-frame hazard** you asked for. Verified rather than restated: `write_block` builds
   its flags as `SUBCARRIER_1 | DATA_RATE_HI` with no `ADDRESSED` flag and no UID, `inventory` is
   1-slot, and nothing ever sends STAY QUIET. So a bystander tag gets zeroed — *and* it can win the
   post-wipe inventory. That second half matters more than it first looks. The "UID changed" screen
   exists for a gen1 card whose UID the wipe rewrote, since the zeros land in blocks 56/57, which *are*
   the UID registers: the card still works but no longer answers to the identity its owner recorded, and
   the screen printing the UID it answers to now is the only route back to it. If a bystander wins that
   inventory, the screen prints the bystander's UID instead — so the owner records an identity belonging
   to a different card, and the real one is never shown.
2. #252 — **Gen2/Classic and USCUID-UL never report a card removed mid-write**, because they discard the
   activation error that says it has gone. Gen4 and Gen1A are the contrast cases that isolate the
   cause, and the debug log shows the state machine stopped rather than grinding. There is a precedent
   for the fix in the ISO15693 poller this PR adds: it counts those same errors against
   `ISO15693_POLLER_MAX_ACTIVATION_ERRORS` and reports `CardLost`.
3. #253 — **on Gen2/Classic, Back during a write is inescapable.**
   `nfc_magic_scene_gen2_write_check_on_enter` pushes the write scene from `on_enter`, so Back pops to
   the check scene which immediately pushes forward again. Each trip round the loop also frees and
   reallocates the poller, so re-applying the card silently restarts the write from block 0 rather than
   resuming.

## Not in this pass

Kept separate as you asked. The next pass takes your three worth-fixing items — the positional "Card too
small" claim, the re-probe classifying by presence, and the unclamped `block_size` going into the
32-byte stack buffer — plus the seven documentation-drift items under your ownership model. The
simplification pass follows: the duplicated retry loop first, since you called that one non-optional and
it has already shipped a bug once, then the result-scene table and the duplicated confirm scene.

---

# Reply in thread: `magic/protocols/iso15693/iso15693_poller.c:897`

**His comment:** Blocking 1 — the read-back can't see the change it exists to catch  
**Status:** FIXED  
**Thread id:** `3719022267`

Fixed in `ad4272c4`. New `Iso15693WriteStateVerifyWipe`, entered after `NfcCommandReset`, exactly as you suggested and the way `VerifyGen2` already does it.

You were right that this shipped at the wrong severity. Our own pre-push review raised the missing power-cycle and rated it **minor** *because* it was gen1-only and therefore untestable. That's backwards — the commit message, the CHANGELOG and the consent screen all rest on this check, so an unverifiable one is worth less than none. Untestability should have raised the severity, not lowered it.

Three consequences of the state split worth flagging, since the inline read didn't have them:

- **A wipe that cleared nothing skips the power-cycle entirely** and Fails as before. No write landed that could have moved the UID, so there is nothing to verify.
- **If the card doesn't come back from the reset, the wipe result is still reported**, not `CardLost`. A card that fails to return is the same non-observation as an inventory that fails, which `VerifyWipe` already logs and ignores — so it must not discard a result the sweep has already earned. Without this, lifting the card the instant the wipe finished would throw the whole report away.
- **That wait has its own activation budget** (15, ~1.5s) rather than the shared 40 (~4s). Behind a reset the result can't be reported until the card returns, so on the shared budget the common lift-immediately case would sit on a frozen popup for four seconds. Not cut to the minimum either: giving up early would skip the UID check on precisely the card the check exists for.

Benched on the gen2 card: no `wipe: card did not return after the field reset` (so the card returned inside the budget and the verify genuinely ran) and no `wipe: the UID CHANGED` (it verified identical — the gen2 regression test you asked for). The power-cycle costs ~250-300ms; the log brackets a full activation of that card at under 170ms plus the ~100ms field-down.

---

# Reply in thread: `magic/protocols/iso15693/iso15693_poller.c:730`

**His comment:** Blocking 2 — the sweep can cover less than the loop it replaced  
**Status:** FIXED  
**Thread id:** `3719022269`

Fixed in `cfa1b37f`. The `break` is now gated on the whole advertised range having been attempted. The tail-drop rule is untouched.

You were right that I conflated two separable decisions — and I put that wrong adjudication into a commit message and into my reply to you. Two reviewers disagreed, I picked a winner, and the actual answer was that the question needed splitting.

One precision on the phrasing: the gate is `block + 1 >= advertised`, not `block < advertised`. `block` is the index just attempted, so blocks `0..block` have been covered and the advertised range is complete at `block == advertised - 1`. Written the other way the sweep defers one block further than it needs to.

The card-present check moved with the break, since a lifted card and a dead stretch are indistinguishable from the write results alone. But it also re-runs once per run length while the sweep is below the advertised count, so a removal is still caught within ~8 blocks rather than after the entire claimed range — otherwise a mostly-dead card would pay a retried inventory per block.

Re-verified the fake-flash trace: 66 advertised against 64 physical trips at block 71, where the break is now permitted, drops 64..71, and reports a clean Success at `blocks_total = 64`.

Bench evidence for the case you described, from a card advertising 28 against 64 physical — the original residue configuration:

```
wipe: card ends at block 63 (advertised 28)
wipe: 72 blocks attempted, 64 cleared, 907ms (advertised 28)
```

72 attempted is 64 real blocks plus the 8-block absent run that ends the sweep. The claim was ignored, the real top was found, all 64 cleared.

---

# Reply in thread: `magic/protocols/iso15693/iso15693_poller.c:786`

**His comment:** Blocking 3 — a truncated sweep and a complete one are indistinguishable  
**Status:** FIXED  
**Thread id:** `3719022271`

Fixed in `241d3957`. New reason on the result scene:

```
Wipe complete
Cleared 64 blocks.
Card claims 28.
```

Both figures, no verdict — `blocks_total < advertised` is deliberately **not** flagged as an error, since that would re-break the fake-flash card, which is undamaged.

Benched at three geometries, which happened to cover all three cases of the screen:

| card advertises | physical | screen |
|---|---|---|
| 64 | 64 | `Cleared 64 blocks. / Card claims 64.` |
| 70 | 64 | `Cleared 64 blocks. / Card claims 70.` |
| 28 | 64 | `Cleared 64 blocks. / Card claims 28.` |

This does add an eleventh render branch to that scene — see my reply on your render-branch comment, where I've set out why it's the one shape the table you proposed absorbs for free.

---

# Reply in thread: `magic/protocols/iso15693/iso15693_poller.c:657`

**His comment:** No wall-clock bound on the sweep  
**Status:** FIXED  
**Thread id:** `3719022293`

Fixed in `78e1d168`. A `furi_get_tick()` deadline, checked before each block so the loop variable stays the exclusive end of the attempted range for the tail arithmetic.

**10s**, set as a backstop rather than a tuning knob, because the two errors it sits between are wildly asymmetric: cutting a legitimate sweep early leaves real data unwiped above the cut — this sweep's own privacy failure, reached from the other direction — while an over-long sweep only makes the user wait, and the block ceiling already caps that near 18s. So it clears any sweep a real card can ask for by a wide margin rather than being tuned down to shorten the pathological case.

The sweep also logs what it cost, in the same commit, so the constant has evidence behind it rather than an estimate:

```
wipe: 72 blocks attempted, 64 cleared, 907ms (advertised 28)
```

Most of that 907ms is the expensive tail — 8 refused blocks at 3 write attempts plus 10ms of waits plus a read each, then 8 re-probe reads — which puts a successful zero-write in the low single-digit milliseconds. So the largest sweep a card could legitimately ask for, 256 blocks all accepting, is ~2s against the 10s bound, while the case you identified (a card answering reads at every address, ~50ms/block) lands near 13s and gets cut.

A cut sweep says so rather than passing its range off as the card's extent: `Wipe stopped / Cleared N blocks. / Card claims M. / Time limit reached.`, plus a `Stopped: time limit` qualifier on the partial screen that outranks the other three, since it changes what the counts mean.

Note this pairs with swallowing Back during a write (`65d71105`, scoped to ISO15693 — reasoning in the top-level reply): that stops the un-abortable window freezing the GUI, this stops it being 18 seconds long.

---

# Reply in thread: `CHANGELOG.md:85`

**His comment:** Stale “and the UID is unchanged”  
**Status:** FIXED  
**Thread id:** `3719022325`

Fixed in `a3e13a3a`. It now reads "a wipe failure saying no block accepted the zero-write" — your suggested wording.

Fixed in this pass rather than held for the documentation batch, since it sits in the same list the new entries were joining and leaving a known-false line while editing its neighbours seemed worse than the scope creep.

---

# Reply in thread: `magic/protocols/iso15693/iso15693_poller.c:762`

**His comment:** Re-probe classifies by presence, main loop by content  
**Status:** NOT fixed — replying because this round made it worse  
**Thread id:** `3719022273`

Not fixed in this pass — it's in the next batch with the other two worth-fixing items, keeping this pass to the hazards.

But you should know that **the blocking-2 fix widened this one's exposure**, so it's worth re-weighing rather than carrying forward at the same priority.

Not breaking below the advertised count means the re-probe can now run over a much longer run when the sweep crosses that boundary — where before it only ever covered ~8 blocks near the card's top. So a block that answers on re-probe while reading back as zeros gets counted "not cleared" more often than it used to. Still fails closed, as you noted, but the false-report rate went up rather than down.

The fix is unchanged from what you described — reuse the `has_data` scan from the main loop — I just didn't want it landing silently in the same pass as the hazard fixes.

---

# Reply in thread: `scenes/nfc_magic_scene_iso15693_write_fail.c:131`

**His comment:** “10 render branches”  
**Status:** NOT fixed — replying because it’s now 11  
**Thread id:** `3719022322`

Not touched in this pass, per your request to keep the simplification separate from the hazard fixes.

One thing you should know before you count them again: **it's 11 now, not 10.** The wipe success screen (`241d3957`, your third blocking item) added a branch.

That was deliberate rather than careless. It's the exact structural sibling of `over_capacity` — a qualified success with counts, title plus a body built from a `FuriString` — so the `{reason, title, body}` table you proposed absorbs it with no new shape. But it does mean the collapse got marginally bigger because of this round, and I'd rather say so than have you find an extra branch where you left ten.

---

# New issue

**Title:** NFC Magic ISO15693: writes and inventory are unaddressed, so a second tag in the field is written too

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.
>
> Two things to flag before filing. This concerns the **ISO15693 support added in #250**, which is not
> in a released version yet — so "App version" is the unreleased 2.1, not 2.0. And the reproduction
> below is **expected from the frame construction, not staged on hardware**: it needs two ISO15693 tags
> in the field at once and would destroy data on the second one.

### App

NFC Magic

### App version

2.1 (unreleased — the ISO15693 support in #250)

### Describe the bug

The ISO15693 wipe sends its `WRITE BLOCK` frames **unaddressed**, so every ISO15693 tag inside the
reader field receives them — not just the one the user selected. A second tag in range has its blocks
zeroed too, with nothing in the UI indicating another tag was ever there.

Both of the helpers involved are unaddressed broadcasts, and nothing suppresses the other tags:

- `iso15693_3_poller_write_block()` builds its request flags as
  `ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI` — no `ADDRESSED` flag and no
  UID in the frame.
- `iso15693_3_poller_inventory()` sends a **1-slot** INVENTORY (`INVENTORY_T5 | T5_N_SLOTS_1`), so with
  two tags present it returns whichever wins the slot rather than detecting the collision.
- The poller never sends STAY QUIET, so nothing puts a bystander tag out of scope.

The same flaw then undermines the report. The wipe re-reads the UID afterwards to catch a card whose
identity it moved — on a gen1 card the zeros land in blocks 56/57, which *are* the UID registers, so the
card carries on working but stops answering to the identity its owner recorded, and the screen printing
the UID it answers to now is the only route back to it. If that post-wipe inventory is answered by the
**bystander** instead, the screen prints a UID belonging to a different card entirely: the owner records
the wrong identity for their own card, and the real one is never shown. Strictly worse than printing
nothing.

Needs two ISO15693 tags within the field simultaneously, which is uncommon but hardly exotic — a badge
holder or wallet does it.

### Reproduction

**Not staged on hardware** — expected from the frame construction above. It would need two ISO15693 tags
and would destroy data on the second, so it has not been run:

1. Place two ISO15693 tags within the reader field at once.
2. NFC Magic → the ISO15693 menu → **Wipe**.
3. Expected: both tags are zeroed, though only one was presented for the operation; and the post-wipe
   UID check may report against whichever tag answers the inventory.

### Firmware version

Momentum `mntm-012-308-g8ed809fba`, not Unleashed. Local build carrying unrelated LF RFID changes; none of them touch `lib/nfc`, `furi_hal_nfc` or `applications/main/nfc`, so the NFC stack is stock Momentum.

### Anything else?

**Possible directions.** Not prescribing an approach, but for discussion:

- use the **addressed** form (`ADDRESSED` flag + UID) for `write_block` once activation has established
  a UID — the wipe already knows the target's UID before it writes anything;
- and/or send **STAY QUIET** to any non-target tag after inventory;
- and/or run a **16-slot** inventory first and refuse to write when more than one tag answers, which is
  the cheapest option and fails safe.

The last one alone would close the destructive half.

**On which tracker this belongs to.** The two helpers are firmware SDK code
(`lib/nfc/protocols/iso15693_3/iso15693_3_poller_i.c`), not app code — but the destructive behaviour is
the app's wipe, and every fix above is available app-side, since the app already builds its own raw
frames for the magic commands. Filing here for that reason; a firmware-side report may be warranted
too, as any caller of those helpers has the same exposure.

**Where.** Reached from `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c`
(`iso15693_poller_wipe_blocks`, and the UID verify in `Iso15693WriteStateVerifyWipe`).

Raised by @mishamyte during review of #250 and split out at his request for future work.

---

# New issue

**Title:** NFC Magic: a card removed mid-write is never reported on Gen2/Classic or USCUID-UL — the poller stalls waiting to re-activate

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.

### App

NFC Magic

### App version

2.0

### Describe the bug

Lift the card during a clone and the write never reports anything. The popup stays on
**"Writing / Don't move..."** indefinitely — no "Card removed", no partial result, and no report of the
blocks that *were* written before the card left, which for a half-completed clone is exactly what you
need to know.

Tested across five protocols, lifting the card the moment the popup switched to "Writing":

| protocol | card | needs a re-activation mid-write? | result |
|---|---|---|---|
| **Gen2 / Classic** | CUID magic Classic 1K | **yes** — halts after every block | **no report.** 88s observed, then still nothing |
| **USCUID-UL** | magic NTAG216 | **yes** — resets on a failed page | **no report** after >1 min, frozen at `Writing 147/231` |
| Gen4 | GTU "Ultimate Magic Card" | no — never halts, stays activated | reports immediately |
| Gen1A | magic Classic 1K | no — abandons the write on the first failure | reports promptly |

Gen1A and Gen4 are the useful contrast: the same action produces a proper failure screen there, so this
is not inherent to losing the card mid-write.

On Gen2/Classic there is no escape either, because Back leads to an inescapable loop — a separate
defect, #253. Note Back does not abort a write in any case: the scene's `on_exit` calls
`<proto>_poller_stop` → `furi_thread_join`, so it waits for the worker and discards the report.

### Reproduction

1. Load a saved MIFARE Classic dump (or a MIFARE Ultralight/NTAG dump for USCUID-UL).
2. Start a clone onto a Gen2/CUID magic Classic card, or a magic NTAG that the app detects as USCUID-UL.
3. Lift the card as soon as the popup switches to **"Writing / Don't move..."**.
4. Wait. The popup does not resolve.

### Firmware version

Momentum `mntm-012-308-g8ed809fba`, not Unleashed. Local build carrying unrelated LF RFID changes; none of them touch `lib/nfc`, `furi_hal_nfc` or `applications/main/nfc`, so the NFC stack is stock Momentum.

### Logs

Gen2 clone, `log debug`, at the moment the card was lifted:

```
9282385 [D][GEN2] Block 34 finished, halting
9282389 [E][ISO14443_3A] Sdd response wrong length
9282492 [D][Nfc] FWT Timeout
9282595 [D][Nfc] FWT Timeout
...            <- every ~103ms for the next 88 seconds, and not one GEN2 line
```

### Anything else?

**Why.** `gen2_poller_write_block_handler` halts the card after every block (`gen2_poller_halt`, at its
end), so reaching the next block needs a **re-activation**. Once the card is gone that activation fails,
the iso3 poller emits `Iso14443_3aPollerEventTypeError`, and `gen2_poller_callback` acts only on
`Ready` — so it discards it and the state machine is never called again. The 103ms cadence above is the
`furi_delay_ms(100)` in `iso14443_3a_poller_run`'s error path.

That single rule covers every protocol tested: **a write stalls exactly when it needs a re-activation
after the card has left.** Gen4 discards the same event and is fine, because it never needs one — it
does not halt between blocks, so it keeps receiving `Ready`, fails the write and sets its terminal
state. USCUID-UL needs one for a different reason: its write handler returns `NfcCommandReset` on a
failed page, deliberately, to revive a tag that went mute after NAKing a locked page.

**Possible direction.** Handle `Iso14443_3aPollerEventTypeError` in the two affected callbacks rather
than discarding it — count it against a budget, and on exhaustion emit the terminal event that fits that
poller: `Partial` with the blocks or pages that did land, rather than `Fail`. There is a precedent in
this pack: the ISO15693 poller added in #250 counts activation failures against
`ISO15693_POLLER_MAX_ACTIVATION_ERRORS` and reports `CardLost`.

Any change to USCUID has a trap: that `NfcCommandReset` is deliberate, and a naive budget would destroy
the behaviour it exists for. It has to tell "reset to revive a NAKing tag" apart from "the card is gone".

**Where.** `base_pack/nfc_magic/magic/protocols/gen2/gen2_poller.c` (`gen2_poller_write_handler`) and
`.../uscuid_ul/uscuid_ul_poller.c` (`uscuid_ul_poller_write_handler`). `gen1a` and `gen4` are
unaffected; both were tested.

Found while testing #250, which does not touch any of these files — `gen2_poller.c`,
`uscuid_ul_poller.c` and the write-check scene are byte-identical to 2.0.

---

# New issue

**Title:** NFC Magic: Back during a Gen2/Classic write is inescapable, and silently restarts the write from block 0

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.

### App

NFC Magic

### App version

2.0

### Describe the bug

Pressing Back during a Gen2/Classic write does not leave the write screen. It returns to it, at its
card-search state — **"Apply the same card to the back"** — and pressing Back again does the same. There
is no way out of the loop.

`nfc_magic_scene_gen2_write_check_on_enter` pushes the write scene from **`on_enter`** when the target
has no write problems:

```c
if(problems.all_problems == 0) {
    ...
    scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWrite);
    return;
}
```

It is a pass-through scene but stays on the scene stack, so Back pops to it, its `on_enter` runs again,
and it immediately pushes the write scene forward again:

```
Write --Back--> Gen2WriteCheck --on_enter--> Write --Back--> Gen2WriteCheck --> ...
```

**Second effect: the write silently restarts.** `nfc_magic_scene_write_on_enter` allocates the poller and
`on_exit` frees it, so each trip round the loop is a **brand-new write starting from block 0**, not a
resumption. Lift the card mid-write, press Back, re-apply it, and the clone completes — having rewritten
the whole card from scratch. Any partial-write counts shown afterwards describe the fresh attempt, not
the interrupted one.

### Reproduction

1. Load a saved MIFARE Classic dump.
2. Start a clone onto a Gen2/Classic target that has no write problems, so the check scene passes
   straight through.
3. At **"Writing / Don't move..."**, press Back.
4. The write screen reappears at **"Apply the same card to the back"**.
5. Press Back again — the same screen. There is no exit.

Reachable without removing the card at all: any Back during a Gen2/Classic write hits it.

### Firmware version

Momentum `mntm-012-308-g8ed809fba`, not Unleashed. Local build carrying unrelated LF RFID changes; none of them touch `lib/nfc`, `furi_hal_nfc` or `applications/main/nfc`, so the NFC stack is stock Momentum.

### Logs

Four Back presses during one write, each tearing down and re-creating the poller, then the card
re-applied:

```
9370728 [D][GEN2] Stopping Gen2 poller
9409962 [D][GEN2] Stopping Gen2 poller
9410660 [D][GEN2] Stopping Gen2 poller
9410814 [D][GEN2] Stopping Gen2 poller
9410953 [D][GEN2] Stopping Gen2 poller
9413351 [D][GEN2] Block 0 is the same, skipping   <- restarted from block 0
```

### Anything else?

**Why it is not usually noticed.** The loop needs the write screen to still be on top when Back is
pressed. If the write completes first, the result screen replaces it and the check scene is never
re-entered from behind. It shows up when a write is slow or has stopped reporting — which is exactly
#252, so the two compound: the write does not resolve, and Back does not get you out.

**Possible directions.** The check scene should not remain on the stack when it has nothing to display:

- have the caller decide, so the check scene is only entered when `all_problems != 0`;
- or replace rather than push, so no frame is left behind to re-fire;
- or record in the scene state that it has already passed through, and have `on_enter` return to the
  menu on the way back rather than pushing forward again.

**Where.** `base_pack/nfc_magic/scenes/nfc_magic_scene_gen2_write_check.c` (`on_enter`). Worth checking
`nfc_magic_scene_mf_classic_write_check.c` for the same shape.

Found while testing #250, which does not touch this scene — it is byte-identical to 2.0.
