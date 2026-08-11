# PR #250 — everything queued for posting

Generated from the files in this directory by `assemble-review.py`. Nothing here has been posted.
Push the fork branch **first** — the replies cite fork SHAs.

| # | goes to | source |
|---|---|---|
| 1 | PR #250 top-level comment | `pr-reply-body.md` |
| 2 | thread on `iso15693_poller.c:859` | `pr-inline/3756085157.md` |
| 3 | thread on `iso15693_poller.c:978` | `pr-inline/3756085164.md` |
| 4 | thread on `iso15693_poller.c:532` | `pr-inline/3756085182.md` |
| 5 | thread on `iso15693_poller.c:1125` | `pr-inline/3756085187.md` |
| 6 | thread on `scenes/nfc_magic_scene_write.c:519` | `pr-inline/3756085228.md` |
| 7 | thread on `iso15693_poller.c:741` | `pr-inline/3756085255.md` |
| 8 | thread on `scenes/nfc_magic_scene_iso15693_write_fail.c:344` | `pr-inline/3756085261.md` |
| 9 | thread on `iso15693_poller.c:866` | `pr-inline/3756085272.md` |

---

# 1. Top-level comment on PR #250

Sixteen commits, one per decision. The blocking item is fixed with your discriminator, the three you'd
bundle are in, both simplifications you asked to take alongside them are in, and so are the nine
worth-fixing and drift items still outstanding from the round before.

Details in the threads; three things need saying here.

## Where the contract landed, which is not where you suggested

You asked the Success enum to document that a wipe Success no longer implies full coverage. I've removed
the divergence instead: **a sweep the clock cuts short now reports `Partial`.**

The poller was saying Success while the screen played the error tone, titled itself "Wipe stopped" and
offered Retry. Documenting that would have left every future consumer needing to remember the footnote
to get it right — which is exactly how one screen out of four came to read `sweep_truncated` at all.

The line I drew is whether the operation's own job is left undone. A cut sweep never attempted blocks
the card claims, so the wipe did not do what a wipe is for. The UID check failing to reach an answer is
different: every block the sweep reached is clear, the wipe finished, and only a best-effort identity
check did not run — making *that* Partial would downgrade every wipe where the user lifts the card as it
completes, which is the case you were protecting when you said not raising an error was the right call.

The contract got **shorter** as a result, which I take as the tell that it had been stated in the wrong
place.

## Your fix for the blocking item was one step short

The activation-cache discriminator is exactly right and I've used it as written. But "keep the bit and
count it" alone renders **"Wiped 0/20 blocks, not cleared: 44"**: the partial screen derives its cleared
figure by subtracting failures from the total, so 44 kept failures against a `blocks_total` of 20 breaks
the `failed_count < blocks_total` invariant you verified by hand last round.

A kept block has to update `highest_present` too. That follows from your own argument — if the cache
proves the block exists, it belongs in the count of blocks proven to exist — but it isn't in the sketch,
and the failure is silent.

## What is verified, and what is not

Hardware, on the gen2 card: the fake-flash regression the blocking fix could have broken (70 advertised
against 64 physical still reports a clean 64), a clean wipe unchanged by the truncation rework, a clone
with the card lifted, and both new card-lost exits in the sweep — above and below the advertised count —
now emitting the timing line they used to skip.

Not verified, and I'd rather say so than let it read as covered:

- the blocking fix's **positive** case — a dead stretch inside the claimed range
- every truncated-sweep screen, and truncation reporting as Partial
- `uid_verified` false
- the capacity gate's discriminating case — a failed block that answers a read
- a clock-cut clone with the card still present

All five need a card that refuses a write while still answering a read, which neither of us has. The
capacity gate is the one where that's least satisfying: on our 70-block non-empty-tail source it still
reports "Card too small", correctly — the card genuinely is too small, and the six blocks refuse reads
as well as writes. What changed is that the claim is now *earned* rather than inferred from the shape of
the failure run. The case where it would newly stay silent is the one we can't stage.

I've written up what a bench for this would take — host-side tests of the sweep's decision logic look
more tractable than tag simulation, since the arithmetic is pure over the results of two calls and every
geometry you've traced by hand becomes table-driven. Not started.

## Still to come

The simplification pass: your two remaining minor items (hoisting the tick conversion out of the sweep
loop, naming the `block + 1 >= advertised` off-by-one), plus the queue from the round before — the
duplicated retry loop you called non-optional, the `{reason, title, body}` table, and the duplicated
confirm scene. The render-branch count went up again this round, to twelve.

The comment density is the part I'm least happy with. It went 39.4% to 42.4% before I started measuring
per-commit; the last ten commits are net +73 comment against +91 code, which is the right side of the
line but not by much, against ~10% for `gen2_poller.c` and `uscuid_ul_poller.c`. The cut belongs in that
same pass, under your ownership model.

## One confirmation

Yes, deliberate: a clean ISO15693 wipe no longer auto-dismisses through the shared success popup. It
matches the over-capacity clone route, and it is the only way to show the measured range beside the
card's claim.

---

# Reply in thread: `iso15693_poller.c:859`

**His comment:** Blocking -- tail-drop vs the advertised-count floor  
**Status:** FIXED  
**Thread id:** `3756085157`

Fixed in `c358eb50`, using your discriminator as written: before dropping a run member below `advertised`, check it against the activation cache. Non-zero is one-way proof the block existed and held data, so it stays in the report; zeros drop as before, which leaves the fake-flash card's phantoms dropping and its clean Success intact. Verified on hardware — 70 advertised against 64 physical still reports "Cleared 64 blocks. / Card claims 70."

Your trace reproduces exactly, and the framing that it is the *floor* and the *drop* asserting opposite things about the same blocks is the part I'd missed entirely.

**One step short, though.** "Keep the bit and count it" alone renders "Wiped 0/20 blocks, not cleared: 44": the partial screen derives its cleared figure by subtracting failures from the total, so 44 kept failures against `blocks_total = 20` inverts it and breaks the `failed_count < blocks_total` invariant you verified last round. A kept block has to update `highest_present` as well — which follows from your own argument, since a block the cache proves exists belongs in the count of blocks proven to exist, but it isn't in the sketch and the failure is silent.

The residual is stated rather than coded, as you suggested: a block already dead when the card was presented reads back as zeros here too and drops with the phantoms. Different symptom — degraded before the wipe rather than during it — and closing it needs a second read pass at activation.

Not verified on hardware. The positive case needs a card that stops answering both a write and a read partway through its claimed range, which we can't stage; what the bench confirms is that the fake-flash case it could have broken is intact.

---

# Reply in thread: `iso15693_poller.c:978`

**His comment:** sweep_truncated never reaches the nothing-wiped screen  
**Status:** FIXED  
**Thread id:** `3756085164`

Fixed in `303512b2`. You're right that this is the card the clock exists for and the one screen that never saw the flag — `wiped == 0` short-circuits here before any of the truncation reporting.

One deviation: you suggested `\nStopped: time limit` to match the partial screen. I've used the same `at N of M` form instead, since it fits the same three lines and says how far the sweep actually got, which on this card is the whole question. The body has room for three, so on a cut sweep the cut replaces the prose rather than adding a fourth line that the button box would clip.

The partial screen's own qualifier moved rather than being fixed in place: a truncated sweep now reports `Partial` from the poller and carries its own reason code, so it has a screen that can show both figures without competing for the qualifier slot. Reasoning in the top-level reply.

---

# Reply in thread: `iso15693_poller.c:532`

**His comment:** the clone data pass got the Back swallow but not the clock  
**Status:** FIXED  
**Thread id:** `3756085182`

Fixed in `17acf9c1`, using the sweep's own pattern — one `furi_get_tick()` before the loop, one check at the top of the body.

**Correction to one detail:** a cut clone's unreached blocks are *not* already in the bitmap. Only blocks that were attempted and refused get bits; the ones the cut skipped were never touched by anything. Left alone they would have been counted as written, since the partial screen derives its cloned figure by subtracting failures from the total — so a clone stopped at block 10 of 256 would have claimed all 256 landed. They are now recorded as failures, which is what they are, and Details names them.

Worth noting the effect is narrower than the 6s/19s framing suggests. Those figures assume the card is gone, so every write burns the full FDT with no responder. With a card present the writes are quick and even a 256-block source finishes well inside the budget — so the clock only fires on a departed card, and what it actually buys is reaching the card-present check sooner and reporting CardLost instead of holding the popup.

---

# Reply in thread: `iso15693_poller.c:1125`

**His comment:** VerifyWipe asserts a check it skipped  
**Status:** FIXED  
**Thread id:** `3756085187`

Fixed in `ce8e3755`. A `uid_verified` flag, false on both no-observation branches, rendered as "UID not re-checked." on the wipe screen's third line. Your argument for not raising an error is untouched — that stays.

I took your "keeps the success tone if you want it" and kept it, but only for this case, and the reasoning is worth stating because it splits your two findings apart.

A wipe whose UID check didn't run still did its job: every block the sweep reached is clear. A wipe the clock cut short did not — blocks the card claims were never attempted. So the second is now `Partial` and the first stays `Success` with a note. Making the unverified UID Partial would downgrade every wipe where the user lifts the card as it completes, which is the case you were protecting.

Your `:1194` note that narrowing the activation budget makes that branch easier to hit is right and worth keeping in view, but it argued for saying something rather than for widening the budget, and that's what's changed.

A cut sweep also gets the Retry/Exit buttons a lost card already had, per your `write_fail.c:307` note — it is the one wipe outcome where re-running is the correct action.

---

# Reply in thread: `scenes/nfc_magic_scene_write.c:519`

**His comment:** two supporting sentences in the Back comment don't hold  
**Status:** FIXED  
**Thread id:** `3756085228`

Both corrected in `a02971c0`, and thank you for checking rather than accepting — the conclusion being right made the supporting sentences easy to leave alone.

`gen1a` and USCUID-backdoor: right, they self-terminate, and the reason is the one you give — raw `nfc_start` makes `PollerReady` a poll-cycle tick rather than an activation, so the handlers keep running with no card. They are excluded now for want of testing rather than because they strand, and the comment says so.

The one-block-per-Ready list: also right that it isn't the distinguishing property. The comment now names what actually is — **needing a re-activation partway through a write.** gen2 halts after every block, USCUID-direct resets on a failed page to revive a NAKing tag; both then depend on a re-activation that fails once the card is gone, and that failure arrives as the Error event those callbacks discard. gen4 shares the callback shape and is fine precisely because it never needs one.

That framing came out of the debug log rather than reading, incidentally — 88 seconds with no state-machine activity at all, which is what ruled out the "grinding through remaining blocks" model I'd been working from.

`:521-522` now cites both budgets, including the one this PR added whose exhaustion reports the wipe's result rather than CardLost.

---

# Reply in thread: `iso15693_poller.c:741`

**His comment:** three copies of the block-answered rule  
**Status:** DONE -- asked for in this pass  
**Thread id:** `3756085255`

Done in `4c2e0526`, as one of the two you asked to take alongside the fixes.

`iso15693_poller_wipe_note_present()`, three calls. The re-probe's version passes `still_absent` as the run, so the `+ 1` that used to be folded into its arithmetic becomes the ordinary count of the answering block, and the two names for the same quantity collapse to one — as you predicted.

The guard is `if(block > *highest_present)` per your note, which preserves site 3's behaviour and is a no-op at the other two.

One thing landed on top of it: site 3 now classifies the answering block by *content* rather than presence, which was your re-probe finding from the round before. So that call site is the helper plus an emptiness check rather than the helper plus an unconditional increment.

---

# Reply in thread: `scenes/nfc_magic_scene_iso15693_write_fail.c:344`

**His comment:** the details gate stated twice, already disagreeing  
**Status:** DONE -- asked for in this pass  
**Thread id:** `3756085261`

Done in `d62671c9`, and taken first of everything this round, because the truncated sweep needed somewhere to put its Details entry and adding it to two gates that disagree would have preserved the disagreement.

Your predicate as written, keyed on the reason code, with `on_event` keeping only its card-lost distinction. The latent divergence is gone with it.

The eleven `const bool`s at the top are untouched, per your note that the queued table deletes them anyway.

---

# Reply in thread: `iso15693_poller.c:866`

**His comment:** 'every exit' isn't  
**Status:** FIXED -- and it found a bug  
**Thread id:** `3756085272`

Fixed in `50fdc911`. Both card-lost paths now `break` rather than `return`, so the tail runs and the line is emitted; the caller discards the counters when the card is gone, so the tail arithmetic running for it is harmless.

They also increment before breaking, which keeps the loop variable meaning the same thing at every exit — the capacity break already did, the deadline breaks before attempting, and a full run ends past the last index. Without that the line reports one block short on exactly the paths this comment added it to.

Confirmed on hardware: a wipe lifted mid-sweep now emits the line on both card-lost exits, above and below the advertised count. The check is the gap — the run that trips the presence check is eight blocks, so attempted minus cleared reads eight on that path.
