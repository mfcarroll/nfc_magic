# Round 10 — main reply (DRAFT, not posted)

Post between the `~~~~` markers.

~~~~
## Round 9 addressed, gen1 validated, the self-review run — and a reason not to merge yet

All eleven threads are fixed; replies are on each. Also in this push: the gen1 hardware round, the
self-review you asked for, and a device test session that found something that changes where this
PR stands. That last part is the important one, so it is below rather than buried at the end.

## gen1 is validated across three chips

ST LRi2K at 56 blocks, NXP SLIX at 28, NXP SLIX-S at 40. On every one the four-frame sequence sets
the UID, it reads back, and the original restores byte-identically. The armed-card wipe hazard is
reproduced end to end on the LRi2K: the sweep zeroed 56/57, the identity moved, and the
post-power-cycle re-read caught it and reported Partial.

Three corrections came out of it, all three stated the other way round in the code beforehand:

- **A written UID takes effect immediately.** An inventory in the same field session already returns
  it. There is no power-up latch. The `NfcCommandReset` before each read-back stays, because it
  costs nothing and re-activates the card for a clean read — but it is belt-and-braces, not
  load-bearing, and the comments now say which.
- **"The backdoor registers accept writes without acknowledging" was a parse, not a measurement.**
  62/63 answer with error 0x10, and the client renders that as `( fail )`. What replaced it is
  stronger: on an armed card both register writes are refused and the UID moves anyway, so a poller
  acting on those return values would abort a run that worked.
- **The four backdoor addresses are not memory.** The LRi2K taking writes at 56/57/62/63 while
  advertising 56 blocks was read here as memory above its claim, by analogy with a gen2 card holding
  blocks above its own. Wrong analogy. With the capacity search pushed past those addresses on four
  more cards, every one stops at its advertised count and 56/57/62/63 answer no read, ever. A
  28-block gen1 card has 28 blocks.

The wipe outcome is also worse than #255 says. The UID did not move to another valid identity — it
went to **all zeros**, and an ISO15693 UID must begin `0xE0`, so the card is left with no valid
identity at all. Recovery is byte-identical, and only possible because the app prints the original
UID.

The gen1 consent screen drops its "not hardware-tested" line as a result. That caveat was doing real
work while the path had never been tried on anything — it told the user the risk itself was
untested, which is worth knowing before agreeing to it. Now it has been tried on three chips, so the
line has no information left in it, and what remains on the screen is the blast radius: gen1 sets
the UID with an ordinary WRITE BLOCK into 56/57/62/63, so a tag that is not magic loses those four
blocks. That is the part a user can act on.

## And it moved a threshold in round 9's own fix

The fix for threads 01/02 landed "the sweep misses 56/57 only when the card answers nothing there
**and** claims fewer than 57 blocks". 57 is the threshold for block 56 sitting *inside* the claim,
and it misses the other route: the sweep does not stop at the advertised count, it keeps writing
until `ISO15693_POLLER_WIPE_ABSENT_RUN` blocks in a row answer nothing. A card silent from block A
is attempted through A+7, and a write that lands resets the run — so 56 and 57 both go at a claim of
**49**. Measured against the poller rather than derived:

```
44..48  ->  56 untouched, 57 untouched
49..52  ->  56 ZEROED,    57 ZEROED
```

So the old wording called a card advertising 49–56 safe when the sweep reaches its UID registers.
Two tests now sit on 48 and 49.

That figure is yours, from thread 01, and I took it without checking it. Saying so because you asked
for the same in the other direction. It was invisible until this week: the only gen1 card here was the
56-block LRi2K, where 56/57 are the first two blocks past the claim.

The correction runs toward safety — of the five gen1 cards, only the LRi2K reaches its own UID
registers under a wipe.

## THE DEVICE SESSION, AND WHY I THINK THIS SHOULD NOT MERGE YET

The notes have carried one outstanding hardware item since August: two more gen2 samples arrived a
week after the regression runs, and "re-run the regression five on one of them" never happened. It
has now, and it found a functional limit.

**TI Tag-it HF-I Plus refuses unaddressed WRITE BLOCK.** Same tag, same block, three commands (on
proxmark):

```
hf 15 wrbl      -b 8 -d 11223344   ->  ( ok )
hf 15 wrbl --ua -b 8 -d 55667788   ->  error 1, "The command is not supported"   ( fail )
hf 15 wrbl      -b 8 -d 55667788   ->  ( ok )
```

Addressed works, unaddressed is refused with error 0x01, addressed works again as the control.

[👤 This para 100% human-written.] That is fundamentally different to the behaviour of the only gen2 card I had throughout the start of this project, and means that - as it stands - the app only correctly supports a subset of gen2 cards. It's totally fixable, but is feature work.

**Every ISO15693 write this app sends is unaddressed.** So on that silicon a wipe clears nothing and
reports "Wipe failed", and a clone sets the UID through the gen2 backdoor and then fails every data
block. The app's reporting is correct throughout — it says exactly what happened — but it cannot do
the job on those cards.

It is **not a regression**, and I checked rather than assumed: the only non-comment changes to
`iso15693_poller.c` since the passing August run are `COUNT_OF`, renamed constants, two bitmap
helpers, a moved declaration and `block_is_empty`. Nothing reaches the wire. What differs is the
card. August ran on the EM-Marin sample at IC ref 0x0F, which accepts unaddressed writes; the new
ones are TI Tag-it at 0x8B. I re-ran the regression five on the EM-Marin card as the control and
they still pass.

**This changes what #251 is.** It was filed as a bystander hazard — unaddressed frames reach any tag
in the field. That still holds. But it is now also a **compatibility limit**: some silicon refuses
unaddressed WRITE BLOCK outright, so addressing the writes stops being a safety improvement and
becomes a functional requirement.

The asymmetry that hid it for three months: TI accepts the gen2 backdoor, which is also unaddressed
but a custom 0xE0 command, and refuses the standard WRITE BLOCK. So the card classifies as gen2
magic and the UID write succeeds. Only the data pass fails.

A second, smaller find from the same session. A gen1 clone of a 28-block source onto a 28-block card
reports:

```
Cloned 28/28 blocks / Not written: 0 / gen1: 56/57/62/63 differ
```

Nothing failed, and those blocks are not in a 28-block source to begin with. The correct gate is
already in the tree twenty lines away — the block-count deduction tests `backdoor_blocks[i] <
source_count`, which is why "28/28" is right — and neither screen uses it. Not a comment fix: the
scene cannot derive the answer, so the poller has to record whether a backdoor block was actually
skipped.

Both are deferred rather than patched into this round, and neither is in what you are about to
read. I would rather the diff you review stays what it says it is.

## The self-review

`/pr-review-toolkit:review-pr`, seven agents, over the whole diff rather than the delta. One
user-facing gap, one contract hole, and about twenty prose defects.

**The user-facing one.** A wipe that loses the card never said the identity check had not run. It
returns before the verify state is entered, so `uid_verified` stays false; `has_details` sent
`CardLost` to its default arm, so there was no Details button; and "UID not re-checked" is reachable
only through Details. On an armed gen1 card the sweep has already written 56/57 by then, so the card
can be gone and its identity with it, and the screen said only that it had been removed.

`has_details` now covers `CardLost` when the mode is wipe and the check never answered. Two things
there were not obvious: the existing note blames a field reset that never happens on this path, so
the wording is chosen by route; and the block list is suppressed, because the sweep's own card-lost
check says a lifted card "can surface as a pile of blocks that wouldn't clear". Verified on device
this session.

**The contract hole.** The wipe set its live progress denominator above the geometry guard rather
than below it, so a card reporting block size 0 carried its own unverified claim into a terminal
event. One statement moved eight lines down, with a test.

**What the review did not find** is the more useful half. A full correctness pass found no
functional defect: every alloc/free pair including early returns, buffer and index safety against
card-supplied counts, the tick-wraparound form, the tail-drop arithmetic at all five loop exits, all
thirteen reason codes reaching a titled screen, the right-slot rule agreeing between `on_enter` and
`on_event`, and the Back-swallow argument protocol by protocol.

## What is left, in order

**Shrinking what you have to read, first:**

1. **The comment cut.** Measured rather than guessed: 1,576 comment lines on this surface, 787 of
   them in 43 blocks of ten lines or more, and roughly 250–370 removable by moving argument out and
   keeping constraints and measurements. Two thirds is in `iso15693_poller.c` alone, so it is one
   file's delta. Deletion only, and mechanically verifiable as such — code bytes unchanged.
2. **The simplification pass**, which is separate and smaller. Three real items: the pass-cut is
   written out twice and both copies set the two fields that gate Retry, with half the rationale at
   each site; three saturating `total - bad` subtractions share a constraint that is currently
   explained 600 lines away; and the block-size clamp is spelled two ways that agree only by
   accident. One item I am **declining** and would rather say so than leave you to wonder: the
   twelve `widget_add_string_multiline_element` calls can be wrapped, and I had recorded an
   objection that turns out not to hold, but it touches all twelve branch bodies in the most-churned
   file in the PR to save about twenty-five lines of formatting. Wrong trade at this point.
3. **The release notes.** The 2.3 section is **178 lines against 14 and 19** for the two entries
   before it — half the file, for one feature. Same discipline as the comment cut: what a user needs
   to know about the card in their hand stays, the reasoning behind it goes.

**Then the feature work:**

4. **Addressed writes**, and the gen1 caveat gate with them.
5. **Re-test on device**, including the TI cards that cannot currently be written.

**And at merge:**

6. **The squash message.** Since this squashes, the default body is every commit message
   concatenated, and this round alone is nine of them. I have one drafted to hand you as a comment
   when you are ready to merge, rather than leaving you to write one in the merge box or take the
   concatenation. Not yet — it has to describe the final state, and items 1–5 will change it.

I am doing the cut first, and the reason is not tidiness: at 46% comment that file actively slows
the work, and doing the addressing fix in it and then cutting around the new comments costs more
than cutting first.

Nothing here needs your time before that, unless you disagree with the order or want the addressing
work scoped differently — it is your call whether it belongs in this PR at all or in a follow-up.

Both firmwares warning-free, 116 host tests, format clean.
~~~~
