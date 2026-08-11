Three blocking items, one commit each, plus the two you'd bundle with them. Benched on the gen2 card
only — no full matrix, as you asked.

I've replied in the threads rather than here for the five items this pass closes, so you can resolve
them as you verify: the read-back power-cycle, the sweep floor, the wipe success screen, the wall-clock
bound, and the stale CHANGELOG line. Two more threads have replies despite *not* being fixed this pass,
because this round changed what they're worth — the re-probe's presence-vs-content classification, and
the render-branch count.

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

| protocol | card | on a failed block | result |
|---|---|---|---|
| Gen2 / Classic | CUID magic Classic 1K | carries on to the next block | **no report**, popup sits on "Writing" |
| USCUID-UL | magic NTAG216 | carries on to the next page | **no report** after >1 min, frozen at `Writing 147/231` |
| Gen4 | GTU "Ultimate Magic Card" | abandons the write | reports immediately |
| Gen1A | magic Classic 1K | abandons the write | reports promptly |

The correlation is exact: a poller that abandons the write on the first failed block reports straight
away; one that carries on through every remaining block never gets there in any tolerable time. Gen4 is
the decisive case, since it shares Gen2's callback shape and its `Ready`-only event handling and is
nonetheless fine — so those writes are almost certainly grinding rather than stalled, with Gen2 having up
to 64 blocks left to retry and USCUID 84 pages.

So on Gen2/Classic and USCUID-UL, swallowing Back would leave the user on a popup that never resolves
and that they can no longer leave — recoverable only by rebooting. I'd rather under-deliver on your
approval than ship that.

Two things fell out of testing it, both pre-existing and both filed rather than fixed here:

- Gen2/Classic and USCUID-UL not reporting a card removed mid-write. There's a precedent for the fix in
  your own tree: ISO15693 asks `iso15693_poller_card_still_present()` once on failure and reports
  CardLost rather than blaming the card for blocks it never reached.
- On Gen2/Classic, Back during a write is **inescapable**:
  `nfc_magic_scene_gen2_write_check_on_enter` pushes the write scene from `on_enter`, so Back pops to
  the check scene which immediately pushes forward again. It also means each trip round the loop frees
  and reallocates the poller, so re-applying the card silently restarts the write from block 0 rather
  than resuming.

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

1. **The unaddressed-frame hazard** you asked for. Verified rather than restated: `write_block` builds
   its flags as `SUBCARRIER_1 | DATA_RATE_HI` with no `ADDRESSED` flag and no UID, `inventory` is
   1-slot, and nothing ever sends STAY QUIET. So a bystander tag gets zeroed — *and* it can win the
   post-wipe inventory. That second half matters more than it first looks. The "UID changed" screen
   exists for a gen1 card whose UID the wipe rewrote, since the zeros land in blocks 56/57, which *are*
   the UID registers: the card still works but no longer answers to the identity its owner recorded, and
   the screen printing the UID it answers to now is the only route back to it. If a bystander wins that
   inventory, the screen prints the bystander's UID instead — so the owner records an identity belonging
   to a different card, and the real one is never shown.
2. **Gen2/Classic and USCUID-UL not reporting a card removed mid-write**, written up from the runs
   above, with Gen4 and Gen1A as the contrast cases that isolate the cause. It still marks one thing
   unconfirmed — grinding versus genuinely stuck — and says how to settle it
   (`gen2_poller_write_block_handler` logs every failed block at debug level).
3. **The Gen2/Classic write-check Back loop**, with the silent restart-from-block-0 that follows from
   it.

## Not in this pass

Kept separate as you asked. The next pass takes your three worth-fixing items — the positional "Card too
small" claim, the re-probe classifying by presence, and the unclamped `block_size` going into the
32-byte stack buffer — plus the seven documentation-drift items under your ownership model. The
simplification pass follows: the duplicated retry loop first, since you called that one non-optional and
it has already shipped a bug once, then the result-scene table and the duplicated confirm scene.
