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
  `clang-format` clean. Built as `base_pack/nfc_magic` too, not only as the dev app, since the pack is
  what you actually compile.
- **Size: +256 bytes.** Measured by building `origin/nfc-magic-iso15693` and this branch with the same
  toolchain — 165,216 against 165,472. Absolute figures on our toolchain will not match your 149,248, so
  the delta is the useful number: essentially the new Details strings and one `uint16_t` in the result.
- Host tests: **59, all green** (was 55). Five new cases: the two cut-index traces from your `:117`
  thread, the refused-vs-unattempted division a cut clone's report depends on, an uncut clone leaving
  both fields clear, and a cut clone reaching Partial.
- Commit hygiene audit: **1 line** of genuine intra-batch churn — a comment line touched by two
  consecutive commits, once for the flag rename and once for the wording. The audit also flags 16 lines
  in `partial_details.c`, which are `clang-format` re-wrapping byte-identical text after an indent
  change, not a rewritten decision.
- **Verified on hardware**, re-run against this delta rather than cited from last round, since it
  touched the sweep's tail-drop, both clone loop bounds and the backdoor test.

  The card is a **physically 64-block gen2 magic card**. It advertises 70 only because the 70-block
  source's CFG frame programs that count during the clone — which is the whole reason the sweep cannot
  treat the advertised figure as the card's extent, so the setup is the mechanism under test rather than
  a quirk of our sample. The order below matters for the same reason: the wipe is meaningful *because*
  the clone ran first and left the card claiming 70 while holding 64.

  | check | result |
  |---|---|
  | 70-block source onto the card | `Clone partial` / `Cloned 64/70 blocks` / `Not written: 6` / `Card too small`, **Finish** + Details — so an uncut partial is still non-retryable after `is_retryable` learned to read `pass_truncated` |
  | Details on it | `Blocks not written` / `64 65 66 67 68 69` — all six, so the new list bound does not truncate an uncut run |
  | clone, card lifted mid-write | `Card removed before the write could finish.`, Retry + **Exit** — the new right-slot rule correctly yields Exit where nothing sits behind Details |
  | wipe | `Wipe complete` / `Cleared 64 blocks.` / `Card claims 70.`, Finish only — the 8-block phantom tail still drops after `i < advertised` came out |
  | wipe, card lifted mid-sweep | `Card removed…`, Retry + Exit |

  Also observed, and correct: the wipe's progress popup ends at 70/70 on this card while the result says
  64 cleared. The denominator is the advertised count and blocks 64-69 genuinely were attempted, so
  "70 of 70 attempted" is true — the operator read it as covering the claim, which is what it means.

- **Six things this delta changed are NOT verified, and cannot be by using the app.** They are all gated
  on `pass_truncated`, and a healthy gen2 card never truncates — a 64-block sweep is about a second
  against a ten-second budget. So these are unreachable rather than merely unchecked:
  the "Wipe stopped" screen's new buttons and Back-as-exit; its `Stopped at block N.` line; the clone's
  `Stopped at block N` qualifier; the clone's Details note; the cut-bounded block list; and the new
  `uid_verified` note.

  The host tests cover them, which is what that harness is for, but that is source-level coverage and I
  am not going to call it a rendered screen. Worth noting the string those two screens print is the same
  21 characters as the one it replaced, and neither has ever been rendered, so the width is untested in
  both directions rather than newly risky. Next local build will drop
  `ISO15693_POLLER_PASS_MAX_MS` to ~200, which cuts a healthy card a dozen blocks in and makes all six
  render, then revert. Happy to post what they look like.

## One thing to hold me to

Both figures, since the push carries two groups and they say different things. **Round 5's own eleven
commits are +163 comment against +45 code.** Almost all of it is you asking for facts to be written down
— the prefix property, the budget overlap, the rule the two predicates share — which is the category
that has nowhere else to live, but it is still the wrong direction.

**The push as a whole is +170 comment against −6 code**, because Pass C takes 51 lines out. So the net
is roughly code-neutral, and I would rather you had that number than only the flattering half or only
the damning one.

What neither number excuses is the ratio: `iso15693_poller.c` goes from 40% comment to **43%**, against
~8% for `gen2_poller.c` and `uscuid_ul_poller.c`. That makes the comment cut more important rather than
less, and I am not treating it as optional.

## On the tags

Agreed, and thanks for saying so plainly. Nothing here waits on a gen1 card and I would rather not hold
a merge for one. What one would settle stays exactly where the code parks it: the armed-card wipe, the
unlock/commit reading inferred from proxmark's send order, and a UID re-read that reports a change
without preventing one. Arguments today, measurements later.

More cards are inbound anyway — other gen2 silicon, to check the variety rather than one sample, plus
candidates I believe are gen1 and some ordinary ISO15693 tags. None of them has been near this branch
yet. The ordinary tags are the cheap win of the three: they reach the gen1 opt-in path by failing gen2,
which is the only route to the source-inspection warning and the one place in this delta's dedup work
that the gen2 card cannot exercise.

## On the harness

Understood, and no expectation either way. It is out of this PR regardless. If `base_pack` ever does
grow a test directory, the part worth keeping is the three firmware semantics read out of the source —
and now the two you settled by disassembly, which the bench could not have reached.
