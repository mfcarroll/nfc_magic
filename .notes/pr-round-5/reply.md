# Round 5 reply — DRAFT, not posted

Post as the PR comment body once the push lands. Threaded replies are in `thread-replies.md`.

---

No apology needed on the timing — and thank you for the disassembly. Three of the facts you pulled out
of `firmware.elf` were things this branch had been reasoning about rather than knowing, and one of them
turned out to be load-bearing in a place neither of us had noticed.

All three blocking items are fixed, and both of the questions you answered are settled.

## The three blocking items

**1. `write_fail.c:341` — the button labelled Exit opened Details.** Fixed, and you were right that the
premise was the problem rather than the branch. `on_enter` tested `is_retryable` while `on_event` tested
`has_details`, and the two only ever agreed because the sets were disjoint. Nothing said so.

There is now one rule, written at the right slot in `on_enter` and referenced from `on_event`: **the
right slot means Details whenever `has_details`, and Exit only when it does not.** I took your shape —
Retry + Details for `WipeStopped`, with Back as the exit — for the reason you gave: the layout is the
real constraint, Back already escapes, and the "Exit" button was duplicating it. The
`case WipeStopped: return true` arm now renders a button, so the truncation note it exists to reach
has a route.

**2. `iso15693_poller.c:557` — a cut clone reported blocks nothing was sent to.** Fixed, and this one
was the good catch of the round. You are right that it is my own sentence from `f8eb8164` pointed at the
report instead of the classifier: *those blocks refused nothing, they were never attempted.* I stopped
at the classifier because that is where the fabricated claim was visible; the report was fabricating the
same claim one block at a time and I did not look.

Promoted as you suggested, with one change: the flag is now **`pass_truncated`** in both the instance and
the result. It was `wipe_truncated` on the instance and `sweep_truncated` in the result, and once it
covers a clone's data pass, "sweep" is the same drift in a new place. Once it reaches the report:

- `success_or_partial` already downgraded on it, so a cut clone is Partial for the right reason
  rather than incidentally via its back-filled failures
- `is_retryable` covers it, so Retry is offered — and the blocks above the cut have a **stronger** claim
  on Retry than the ones below, which were actually refused
- Details lists only blocks **below the cut**. The bitmap holds two different things at two different
  addresses and only the lower group is a fact about the card
- the note prints, in clone-specific wording — a clone has no advertised count to measure against
- the partial summary names the cut ahead of its other qualifiers, since it is the only one that
  explains the count above it

The routing in `scene_write.c` is now mode-gated: `WipeStopped`'s screen is wipe-specific down to its
wording, so a cut clone stays on the ordinary partial screen.

**3. `CHANGELOG.md:41-48` — claimed a fault the app hides.** Fixed as documentation, and I agree the code
gap is not closable for exactly the reason you give. Both errors corrected: the fault claim now says a
dead stretch is reported *when the app can tell it apart from memory that never existed*, with the
prefix limit stated plainly underneath; and "were read" is now "read back **non-zero content**", with the
one-directional nature of the test spelled out.

## The four you flagged for the same pass

- **The keep branch vs `wipe_note_present`.** Written into the comment, as you asked, and you are right
  that it is the load-bearing fact of the whole mechanism. The prefix property now appears once, in
  full, at the keep branch, with `wipe_note_present` cross-referencing it — and it is named as
  load-bearing in three places rather than one, since it also bounds the "Not closed" disposition and
  makes the zeroed-entry argument deterministic. I also dropped `i < advertised`: `block_held_data`
  range-checks against the same object, so that was two statements of one bound.
- **`!pass_truncated` suppressing "Card too small".** Comment, not a behaviour change, and I want to be
  explicit that I considered both of your options and rejected the second. Detail below.
- **"Stopped at N" printing `blocks_total`.** Fixed with a `cut_block` field, and the framing had to go
  with it — see below.
- **`uid_verified` on one of three wipe screens.** Moved into Details, the route the truncation note
  already uses. Reachable wherever it can fire: a wipe reaching Partial must have `failed_count > 0`
  (`uid_changed` and `pass_truncated` both route elsewhere), so it always has a Details button. Your
  point about the armed gen1 card is in the comment — the sweep writes 56/57 at *index* 56/57, long
  before any plausible cut, so "Wipe stopped" was offering Retry without saying the check never ran.

## Judgement calls worth naming

**The cut index needed a second fix you did not ask for.** Carrying `cut_block` fixes your first trace
cleanly — a sweep that attempted 55 and proved 50 present now says 55. It does **not** fix the second.
On the read-everywhere card `blocks_total` and the cut are nearly the same number, so swapping the field
still renders "Stopped at 200 of 64". The fraction framing was the other half of the bug. The summary
now names one block index and makes no comparison; Details keeps the comparison and picks its sentence
from which side of the claim the cut fell on.

**Your `:26` and your `:557` are in conflict, and `:557` wins.** You asked me to drop the unused
`instance` from `is_retryable`. Fixing the cut clone requires that predicate to read `pass_truncated`
from the result, so `instance` is now genuinely used and the `UNUSED` is gone rather than the parameter.
Flagging it so it is not read as ignored.

**I kept the guard you queried, and here is the trade.** You are right that `!pass_truncated` lands on
the card it was least meant for: this pass's cost is dominated by failing blocks, the budget buys
roughly 150–250 of them, and a genuinely too-small card *is* a long run of failing blocks — so a source
about 150+ blocks larger truncates and loses the line. Retrying is cut in the same place, so nothing
ever names the cause. That is worse than you stated it.

I left it anyway, because dropping it trades one fabrication for another: the blocks below a cut look
like a top tail whether or not memory resumes above them, so a cut claiming capacity is asserting
something about the user's hardware that the pass never finished testing. Under-claiming leaves a
correct report and an actionable Retry. A clone-specific budget would close it properly, at the cost of
more seconds with Back swallowed — which is #252, so it is not free either. Both trades are now written
down at the guard rather than left for the next reader to mistake for an oversight.

**`ISO15693_POLLER_WIPE_MAX_MS` is now `ISO15693_POLLER_PASS_MAX_MS`**, and its comment says the
derivation is a wipe's 4-byte payload while the clone's is up to 8x that plus the probe — so the value
has to be defensible for the more expensive of the two passes, not the cheaper.

**The backdoor block set had four copies, not three.** Same shape as the is-buffer-all-zero helper last
round: the fourth is in `source_uses_gen1_blocks`. One array and one `is_backdoor_block()` predicate
now. The gen1 write *sequence* stays written out at its call site, where the order is the meaning.

**gen4 checks out as unsafe**, verified against `magic/protocols/gen4/gen4_poller.c` — `nfc_poller_start`
at `:813`, Ready-only dispatch at `:99`, no activation budget, `current_block++` at `:281`/`:358`/`:460`.
It is now in the unsafe set rather than enumerated and dropped. Issues named: #252 and #253.

## What is in this push

Three groups, in history order. Each commit is one decision.

1. **The Pass C refactors you deferred in Round 4** — the duplicated retry loop (your round-1
   non-optional), the hoisted ticks and the named off-by-one (`:711`/`:799`), the success route's two
   locals (`scene_write.c:396`), and the folded confirm scene. Plus one prose correction found while
   verifying that fold on hardware. These were finished before this review arrived and held so they
   would not move code under you mid-review.
2. **One CHANGELOG line declaring that gen3 magic is not supported.** A third generation exists
   (proxmark's `hf 15 csetuid --v3`) keeping its UID in blocks `0x10`/`0x11` with a signature in
   `0x14`/`0x15`. Such a card reports "not a magic tag" today, and a wipe or clone writes over those
   blocks like any other data. Declared rather than handled — supporting it is a feature and a
   third generation in the opt-in ladder, which is not a thing to start five rounds deep. Happy to open
   it as its own issue, although not something I could currently work on as I have no gen3 iso15693 tag.
3. **This round's eleven fixes.**

The two Pass C items still held are the `{reason, title, body}` render table for
`iso15693_write_fail.c` and the comment cut under the ownership model. So is the bitmap set/clear
duplication you listed in Round 4 — I have not forgotten it; it moves a lot of lines in the file you
anchor most comments on, so it belongs with the comment cut rather than with fixes.

## Verification

- Momentum 87.15 and Unleashed 88.2, both from a clean object directory, zero compiler warnings.
  `clang-format` clean.
- Host tests: **59, all green** (was 55). Five new cases: the two cut-index traces from your `:117`
  thread, the refused-vs-unattempted division a cut clone's report depends on, an uncut clone leaving
  both fields clear, and a cut clone reaching Partial.
- Commit hygiene audit: **1 line** of genuine intra-batch churn — a comment line touched by two
  consecutive commits, once for the flag rename and once for the wording. The audit also flags 16 lines
  in `partial_details.c`, which are `clang-format` re-wrapping byte-identical text after an indent
  change, not a rewritten decision.
- **Not verified on hardware, and worth being precise about why.** Every screen this round changed is
  gated on `pass_truncated`, and a healthy gen2 card never truncates — a 64-block sweep is about a
  second against a 10-second budget. So these six are unreachable by using the app, not merely
  unchecked so far:
  - the "Wipe stopped" screen's new buttons, and Back as its only exit
  - its `Stopped at block N.` line, which replaced `Stopped at %u of %u.` (same intended width; unseen)
  - the clone's `Stopped at block N` qualifier line
  - the clone's Details note
  - the cut-bounded block list
  - the new `uid_verified` note in Details

  They are covered by the host tests, which is what that harness is for, but that is source-level
  coverage and I am not going to call it a rendered screen. The plan is to lower
  `ISO15693_POLLER_PASS_MAX_MS` to ~200 in a local build, which cuts a healthy card a dozen blocks in
  and makes all six render, then revert. Happy to post what they look like.

  **Nothing in this delta has been on hardware yet**, including the regression side. The Round 4
  results still stand for the code they were taken against, but this round touched the sweep's
  tail-drop, both clone loop bounds and the backdoor test, so they need re-running rather than citing:
  the 70/64 clone and wipe, both card-lifted exits, and Details on an uncut partial still listing the
  full bitmap. None of that needs a gen1 card. Everything else in the delta is non-render code.

## One thing to hold me to

This delta is **+163 comment lines against +45 code lines**, and the file went from 40% comment to 43%
against ~8% for `gen2_poller.c` and `uscuid_ul_poller.c`. Most of it is you asking for facts to be
written down — the prefix property, the budget overlap, the rule the two predicates share — and those
are exactly the constraints that had nowhere else to live. But it is the wrong direction, and it makes
the comment cut more important rather than less. I am not treating that as optional.

## On the tags

Agreed, and thanks for saying so plainly. Nothing here waits on a gen1 card, and I would rather not hold
a merge for one. What it would settle stays exactly where the code parks it: the armed-card wipe, the
unlock/commit reading inferred from proxmark's send order, and a UID re-read that reports a change
without preventing one. Arguments today, measurements later.

## On the harness

Understood, and no expectation either way. It is out of this PR regardless. If `base_pack` ever does
grow a test directory, the part worth keeping is the three firmware semantics read out of the source —
and now the two you settled by disassembly, which the bench could not have reached.
