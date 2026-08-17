# Round 5 threaded replies — DRAFT, not posted

Reply in-thread on his inline comments. Threads not listed here need no reply (verification-only, or
fully answered by the commit). Post after the push so the commit SHAs resolve.

---

## `write_fail.c:341` — **blocking**, Exit opens Details

Fixed, and you were right that the premise was the problem. There is one rule now, stated at the right
slot in `on_enter` and referenced from `on_event`: the right slot means Details whenever `has_details`,
and Exit only when it does not. I took your Retry + Details shape with Back as the exit.

Checked on the card that the rule holds in the direction you can reach: a card lifted mid-write still
renders **Retry + Exit**, because `has_details(CardLost)` is false. So the fix did not turn every
retryable screen into a Details screen — which was the way it could have gone wrong.

The `WipeStopped` side of it is not reachable on a healthy gen2 card at all; see the verification note
in the main comment for how it gets rendered.

---

## `iso15693_poller.c:557` — **blocking**, the cut clone's report

Fixed, and this was the good catch of the round. You are right that it is my own sentence from
`f8eb8164` aimed at the report rather than the classifier. I stopped where the fabricated claim was
visible and did not look at what the report did with the same blocks.

Promoted to the instance, and renamed to `pass_truncated` in both the instance and the result, since
`sweep_truncated` on a clone's data pass would be the same drift in a new place. Full list of what it
buys is in the main comment. Two new host tests -- an uncut clone leaving both fields clear, and a cut
clone reaching Partial -- plus new assertions on the existing clock-cut test for the
refused-vs-unattempted division the screens now depend on.

---

## `write_fail.c:26` — "`instance` is unused"

It was, and it is not any more — your `:557` thread is why. Fixing the cut clone means `is_retryable`
has to answer for a reason code that carries no truncation in itself: a cut clone stays on the ordinary
partial screen, so `pass_truncated` in the result is the only thing distinguishing it there. The
predicate now reads it and the `UNUSED` is gone rather than the parameter.

Your point about the asymmetry being a useful hint stands, though — it just points the other way now.
Both predicates read the result, and the header on each says what it decides.

---

## `write_fail.c:117` — "Stopped at N" prints `blocks_total`

Carrying the cut index fixes your first trace and **not** your second, which I only noticed while
writing the test for it. On the read-everywhere card `blocks_total` and the cut are nearly the same
number, so swapping the field still renders "Stopped at 200 of 64".

The framing was the other half. `%u of %u` parses as a fraction, so any cut above the advertised count
reads as falling short of a number it had passed. The summary now names one block index and makes no
comparison at all; Details keeps the comparison and picks its sentence from which side of the claim the
cut fell on, because which side it lands on changes what is true.

Both traces are pinned as tests.

---

## `partial_details.c:65` — "Blocks above that were never attempted"

Fixed by the same field, and your second case — a card claiming ~200 while holding 10 — was the worse
of the two, since it was making a factual claim about 170 blocks in the opposite direction from the
truth. The sentence is now bounded by the cut, so "above that were never attempted" is true by
construction rather than by luck.

The block list is bounded the same way, and that is the half our card can check: on an uncut partial it
still prints the full bitmap — blocks 64-69, the six a 70-block source cannot fit onto 64 blocks of
silicon — so the new bound does not clip a run that was never cut.

---

## `iso15693_poller.c:649` — the back-fill and the repeated backdoor test

Four copies, not three. The fourth is in `iso15693_poller_source_uses_gen1_blocks`, which builds the
same list as a local array — the same shape as the is-buffer-all-zero helper last round, where you
counted three and there were four. One file-scope array and one `is_backdoor_block()` predicate now.
The gen1 write *sequence* stays written out at its call site, since there the order is the meaning
rather than incidental.

The redundant bitmap bound is gone from both loop headers, and the clamp now says it is the single
point of truth, which is what the loops rely on.

Regression-checked on hardware rather than trusted — a physically 64-block card carrying a 70-block
source, so it advertises 70 while holding 64. The clone still reports
`Cloned 64/70 blocks` / `Not written: 6` / `Card too small` with all six indices in Details, and the
wipe still reports `Cleared 64 blocks. / Card claims 70.` — the second being the one that exercises the
dropped `i < advertised`, since that card's 8-block phantom tail is exactly what the guard was covering.

The one site the gen2 card cannot reach is `source_uses_gen1_blocks`, which sits behind the gen1 opt-in
and so needs a card that fails gen2. Ordinary ISO15693 tags are on order for it.

On the downstream problem you point at: fixed in the `:557` thread. The back-fill stays — you are right
that it is the correct record — and what changed is that the report can now tell its entries apart from
refusals.

---

## `iso15693_poller.c:687` — `!pass_truncated` vs the card it was meant for

Kept, and it is worse than you stated: retrying is cut in the same place, so a user with a source ~150+
blocks larger than the target never gets the diagnosis at all, however many times they try.

I still think under-claiming is right. Dropping the guard does not recover the claim honestly — it
lets a cut assert capacity from evidence that stops mid-run, and the blocks below a cut look like a top
tail whether or not memory resumes above them. That is the same fabrication in a different costume. A
clone-specific budget would close it properly and costs seconds during which Back is swallowed, which
is #252 pulling the other way.

Both trades are now written at the guard, with the arithmetic, so the next reader sees a decision rather
than an oversight. If you would rather have the longer budget I will take that instead — it is a
one-line change and I have no strong claim on 10s over 20s beyond #252.

---

## `iso15693_poller.c:855` and `:993` — the disassembly

Thank you for both. The prefix property is now written into the keep branch in full, and named as
load-bearing in the three places it actually carries: it bounds that divergence, it is why the
"Not closed" note is a disposition rather than a placeholder, and it is why the changelog claim had to
come down.

The `pvPortMalloc` clause is in at the read-back note. You are right that the bench pass could not have
distinguished "the fix works" from "this heap happened to be clean" — `block_held_data` is only
deterministic because the allocator zeroes, and that was resting on an unstated assumption.

I took the smaller of your two options — reconcile in the comment rather than restructure the loop —
because the restructure moves the boundary decision into a new place and this file is already the one
you anchor most comments on. The `i < advertised` guard went either way.

---

## `CHANGELOG.md:42` — the separate consideration

On the main point, agreed and fixed; the code gap is not closable and the changelog now says what the
code does.

On the part you marked as my call — `blocks_total < advertised` being fake flash *or* dead memory, with
the app resolving toward reassurance — you are right, and I have **not** changed it this round.
Deliberately: it is a new behaviour change on a screen that is hardware-verified, and it would arrive
in a delta that is already large.

What I would do, if you want it: use that free third line when `blocks_total < advertised` to say the
shortfall was not explained, rather than leaving "complete" and the success chime to speak for it.
Something like "Some claimed blocks did not answer." It stays out of the `>` case, which is
unambiguously benign. Say the word and it goes in the next push with a hardware check; otherwise I will
file it so it does not evaporate.

---

## `scene_write.c:539` — gen4

Checked and you are right on every part. `gen4_poller_start` uses `nfc_poller_start` (`:813`), the
callback acts only on `Iso14443_3aPollerEventTypeReady` and returns `NfcCommandContinue` for everything
else (`:99`), there is no activation-error budget, and it advances one block per Ready (`:281`, `:358`,
`:460`). It is in the unsafe set now.

The comment that this one replaced did name it, which is the annoying part — the rewrite that existed to
correct the claims dropped one.

---

## `scene_write.c:546` — "the linked issue"

Named: #252 for the 88-second measurement, #253 for needing a reboot. Agreed that a bare reference is
useless from a source tree, which is where the comment gets read.

---

## `iso15693_poller.c:155` — the constant's derivation

Renamed to `ISO15693_POLLER_PASS_MAX_MS`, since it bounds both passes and "WIPE" was the same drift the
truncation flag had. Its comment now states that the arithmetic under it is a wipe's 4-byte zero write,
that a clone's payload is up to 8x that plus the probe, and the rule that follows: whatever the value
is, it has to be defensible for the more expensive of the two passes.

---

## `iso15693_poller.c:618` — the appended rationale

Substituted rather than stacked, as you asked. The read-answers-therefore-exists argument between them
is untouched. The framing is written once and now states the distinction that actually holds: emptiness
decides which *bucket* a failure lands in, not whether the probe is worth paying for.

---

## `iso15693_poller.h:24` — the Success contract

Both overreaches fixed, and the `nfc_magic_app_i.h:129-130` echo with them. The entry now claims the
weaker thing that is true — the run was not cut, and nothing it reached is known to still hold data —
plus the guarantee that does hold: a block proven to hold data is never dropped.

"No block refused" was the worst of the three, since it contradicted the classification table one file
away: a block that refuses every write and reads back empty is deliberately not counted.
