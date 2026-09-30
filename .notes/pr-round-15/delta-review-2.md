# Round 15 — the second delta review, of fold 10

Brief: NEXT-SESSION.md, 2026-09-30. Fork `bd807e5b` against `backup-pre-fold10-replay` (`390a6621`),
dev `ba0b63ee`. Single-threaded, report-only, no agents. Passes 2 and 3 of REVIEW-PROMPT.md, scoped
to fold 10, read in context, plus the twin hunt. Two findings were reproduced with a scratch probe
that `#include`s the existing host tests (nothing in the repo changed).

Five findings, none blocking. mfcarroll picks.

## Findings, most severe first

**1. A clone's lost-card Details can blame the clock (05; `partial_details.c:84`).**
Fold 10 gave a clone's CardLost a Details page. The time-limit note at `:84` is gated only on
`pass_truncated`. On a lifted card each remaining block times out, so on a big enough file the clock
cuts the pass before the card-present check, and the run reports CardLost with both `uid_recheck`
and `pass_truncated` set. *Probe:* gen2 card, lifted at op 40, file of 150, 200 or 256 blocks: CardLost,
`uid_recheck` 1, `pass_truncated` 1, cut at 113. With a 64-block file the lift is found before the cut,
so the note does not appear.
*What the user reads:* "Card removed before the write could finish", then under Details "Clone hit its
time limit at block 113 ... counted as not written, but the card did not refuse them. Retrying may
get further ... the card is too slow to finish in one pass rather than refusing." It blames the time
budget and the card's speed when the card was simply lifted, and it qualifies a count the card-lost
screen never shows. The same page's AFI/DSFID note has the same exposure, but only in the few
milliseconds between VerifyGen2 and the identity read-back.
*Fix:* skip the truncation note (and the identity note) when `card_lost` is set, by the rule
`list_upto` already follows: that exit's counters are the caller's to discard. On the re-read route,
where the clock cut a card that was still present, the note would be true, but its count is not on
screen and Retry covers it. Test: set `r.pass_truncated = true` in
`test_clone_card_lost_after_56_offers_details_for_the_uid_note` and require that "time limit" is absent.

**2. After a conversion at 57, the progress figure still reads one over on a cut pass (05;
`poller.c:1272-1296`, `:1377`).** `done--` takes back the write that moved the UID. At 57, though,
`done` has already counted 56, which the run wrote as memory and the conversion then deducts.
finish_progress's special case covers that only on a finished pass. *Probe:* a gen1 card with all
three attempts at 56 lost and the pass cut anywhere above 57: `done` is one more than the blocks the
total counts, e.g. cut at 62 gives 61 against 60. On a 64-block file (total 60) that is **61/60** on
screen. Before 07 this state needs the rare preconditions: a gen1 card already wearing the target, three
lost attempts, and the clock.
The 05 message says the fix stops "a pass the clock cut above them" reading "one past its own total",
which holds only for a conversion at 56. The test's name, `counts_the_blocks_its_total_does`, also
covers only 56.
*Fix, either:* (a) note a conversion at 57 in the branch and store `clone_blocks_done = done - 1` after
the loop, touching only the final figure. **Not** a `done -= 2` inside the loop: 57 has already been
reported, so the next report would go backwards and could re-emit a band, which breaks the
structural STEPS+1 bound that report_progress's comment relies on. Test: drop mask 0x7 and a cut
above 57, then `done == cut - 4`. Or (b) keep the code, and say in the finish_progress comment and in
05's message that a cut pass after a conversion at 57 reads one over.

**3. Three places still say "geometry" where the note compares block count and IC ref (06; reply).**
The twins of the first review's finding 6, which fold 10 did not touch:
- `nfc_magic_scene_write.c:407`: "geometry is recorded as two independent halves". The halves are
  block count and IC ref, and IC ref is not geometry. Better: "the reported block count and IC
  reference are recorded separately".
- `write_fail.c:265`: "then geometry, which is only how the copy presents". Better: "then the
  reported block count or IC reference".
- `reply.md:144-146`: "so the geometry note compares the block count alone". It also compares the IC
  ref. What the bench showed is that it compares the count, **not the block size**. `:151`, "for the
  geometry", is better as "for the block-count note".
All cosmetic. The two code lines are 06's.

**4. The notes put "Details says the UID was not re-checked" under the gen1-conversion bullet only
(09 CHANGELOG `:123-124`, reply `:80-81`, squash `:101-102`).** The note fires on *every* clone whose
pass sent 56/57 a frame, and that is mostly gen2 cards, the common case. A gen2 user who lifts a
64-block clone late sees a note the release notes placed under gen1. The note's own wording is fine
there. A sentence in the "A card lifted mid-write says so" bullet would cover it; placing it only
under gen1 narrows the note's scope rather than stating anything false. mfcarroll's call: leaving
it is defensible.

**5. 07's message: "It is not new machinery." now reads as describing the verify (07 message).**
That paragraph was written about the re-address. Fold 10 put the two-UID paragraph in front of it,
and the rule it describes does add a helper, `iso15693_poller_uid_is_half_written`. Better: "The
re-address is not new machinery ...". Optionally add that the verify's two UIDs come from the same
predict_uid. Message-only (`.msg`).

## Checked, and held

- **The gen1 two-UID rule (07).** Traced against `send_backdoor_uid_gen1`: on the opt-in path
  `address_uid == original_uid` when 56 is predicted. 56 landing and 57 lost (readdress took the
  address, missed, or was answered by a bystander) leaves half A. 56 lost with 57 landing at the
  original address leaves half B. Those are exactly the helper's two predictions. When the original
  and the target share a half, one prediction becomes the original and the other the target, as the
  comment says. Both are ruled out first, so every other UID is a stranger's. No real outcome loses
  `uid_unexpected`. The only UID rejected that is not a bystander's is one staged by an EARLIER
  session and applied at the Reset, which is the fake's fixture. The three chips measured do not
  latch, so the case is theoretical, and it would get the gen1-failed screen, not a false one. A
  half-written card is proven gen1, so losing the "56/57/62/63 may differ" warning there is correct.
- **The lost-card note (05).** It is true on all three routes: the mid-pass check, VerifyClone's
  inventory miss, and the activation budget after the Reset. A card lifted at block 34 still has
  56/57 sent their frames, and the app cannot know when it left, so "unknown" is honest. On a gen2
  card, "which on a gen1 card are its UID" is conditional, and the app cannot rule out a gen1 card
  wearing the target. What else the page shows there: the gen1 note (true: a conversion proves gen1);
  the survey notes only on the re-read route, where the survey finished on a card that was present
  (on the mid-pass route it never runs and its fields are reset per run); the block list suppressed.
  `uid_recheck` is reset per run, and CardLost always fetches the result, so it is never stale.
- **has_details against is_retryable, and the three-way right slot.** A clone's CardLost with
  `uid_recheck` gets Retry + Details, and Details routes to PartialDetails. Without it: Retry + Exit.
  A Write UID's CardLost: Retry + Exit (`uid_recheck` is never set there). A wipe is unchanged.
  on_event decides in the same order.
- **Accounting (05).** `done--` keeps report_progress monotone: the next report repeats the last
  value. `taken_back`: 0 on gen2 (`converted` false). 0 at a conversion at 56, since the moving write
  `continue`s before any failure is recorded. 56's one failure at 57. The back-fill skips the four
  only once `skipping` is set, which is right on both paths. The finished-pass special case is still
  needed (finding 2).
- **Twins of `uid_unexpected`.** The three sites plus 07's and 05's messages all list three routes.
  The "proves magic" claims hold on every route modulo #251, as they did for the gen2 one. The
  start_*_gen1 headers' "Fail (the gen1 UID didn't take)" still reads correctly.
- **Messages.** 06 and 09 match their diffs. 10 and 13 differ in context only (range-diff). The 2.3
  count was re-derived at 163 of 344. 05's tree names nothing a later commit adds (no `begin_note`, no
  half-written helper).

## Checked, and what came of it

Each finding was checked against the code before anything changed, 2026-09-30, and all five held.
Findings 1 and 2 were re-run with scratch probes of my own.

- **1** reproduces at 20-25 ms a radio operation, about what the loop's own comment implies for a
  lifted card: a gen2 clone lifted early on a file of roughly 120-140 blocks or more ends CardLost
  with both flags, and a 64-block file never does. The wipe has the same exposure, and has had it
  since the pushed build: a lost wipe carries the cut only when the lift lands just before the clock
  runs out, one lift point in 56 in the probe, on a card that refuses writes and answers reads.
  **mfcarroll: both modes.** The time-limit and AFI/DSFID notes are left off any lost card, the rule
  the block list already follows.
- **2** reproduces: every cut pass after a conversion at 57 reads one over, 161 against 160 on a
  200-block file. It passes 100% only on a 64-block file cut at 62 or 63, in the popup's last frame.
  **mfcarroll: fix it.** 56 comes off the final figure after the loop. finish_progress's special
  case for a finished pass now shows what the figure would anyway, and stays, its sentence no longer
  claiming the two differ.
- **3** held. Both code lines are 06's, worded this way first at its sync point, so the fold puts
  them there. The reply's two lines are fixed with them.
- **4** held. **mfcarroll: the release notes only.** The sentence moves from the gen1 bullet to the
  lifted-card bullet, where it covers every clone whose pass sent 56/57 a frame, and the entry is
  still 163 lines of 344. The reply and the squash message describe 05, where the note arrived with
  the gen1 repair, and stay.
- **5** held. The paragraph already followed the verify's before fold 10, but read false only once
  the verify gained a helper. Fixed in 07's message.

**Fold 11** put each fix at the sync point that owns it: 1 and 2 at 05, 3 at 06, 4 at 09, 5 in 07's
message; 05's and 09's messages say what they now carry. One host test is new for each of 1 and 2,
and 2's existing test gained the conversion at 57: 207 at the tip. Six mutations, each caught: either
gate removed, the time-limit gate narrowed to a clone (the wipe run catches it), the take-back after
the loop removed, the take-back applied at either conversion, and the in-loop form the review warned
against. That last one passes every count, and only the new test catches it, by checking that the
progress figure never goes backwards and stays within STEPS + 1 events. Nothing had pinned that bound
before.

**Verified** as fold 10 was: all 29 rewritten commits that touch shipped files or tests pass the
per-commit check, the fold pairs 249 commits with 0 problems, a test replay equals a full sync of the
last anchor and fast-forwards the pushed base, churn stays at 6, the writing gate finds nothing, and
the FAP builds clean on Momentum and Unleashed. A bench is optional: the one visible change needs a
file of about 140 blocks or more lifted early, and a 64-block lift like the one benched on fold 10
reads the same.
