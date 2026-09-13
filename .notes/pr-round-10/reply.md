# Round 10 — main reply (DRAFT, not posted)

Post between the `~~~~` markers.

~~~~
## Round 9 addressed, the gen1 round is in, and I ran the self-review

All eleven threads are fixed; replies are on each. Two of them moved further than the thread asked,
and both are noted there rather than here.

Also in this push: the **gen1 hardware round**, and the **self-review** you asked for.

## gen1 is validated on five cards across three chips

ST LRi2K at 56 blocks, NXP SLIX at 28, NXP SLIX-S at 40. On every one the four-frame sequence sets
the UID, it reads back, and the original restores byte-identically. The armed-card wipe hazard is
reproduced end to end on the LRi2K: the sweep zeroed 56/57, the identity moved, and the
post-power-cycle re-read caught it and reported Partial. The mitigation this PR ships works on real
gen1 silicon.

Three corrections came out of it, all three stated the other way round in the code beforehand:

- **A written UID takes effect immediately.** An inventory in the same field session already returns
  it. There is no power-up latch. The `NfcCommandReset` before each read-back stays — it costs
  nothing, it re-activates for a clean read, and gen2's UID is in a register space this was never
  tested against — but it is belt-and-braces, not load-bearing, and the comments now say which.
- **"The backdoor registers accept writes without acknowledging" was a parse, not a measurement.**
  62/63 answer with error 0x10, and `hf 15 wrbl --ua` renders that as `( fail )`. What replaced it
  is stronger and does not need silence: on an armed card **both** register writes are refused and
  the UID moves anyway, so a poller acting on those return values would abort a run that worked.

- **The four backdoor addresses are not memory.** The LRi2K taking writes at 56/57/62/63 while
  advertising 56 blocks was read here as writable memory above its claim, by analogy with a gen2
  card holding blocks above its own. Wrong analogy. With the capacity search pushed past those
  addresses on all four new cards, every one stops at its advertised count and **56/57/62/63 answer
  no read, ever**. They are write-only registers the magic silicon decodes as a UID-set command. A
  28-block gen1 card has 28 blocks. A gen2 card's over-claim is a different phenomenon and the
  release notes now separate them.

The wipe outcome is also worse than #255 or the code said. The UID did not move to another valid
identity — it moved to **all zeros**, and an ISO15693 UID must begin `0xE0`, so the card is left
with no valid identity at all. Recovery is byte-identical via `hf 15 csetuid`, and only possible
because the app prints the original UID. That screen earns its place.

## And it moved a threshold in round 9's own fix

The fix for threads 01/02 landed "the sweep misses 56/57 only when the card answers nothing there
**and** claims fewer than 57 blocks". 57 is the threshold for block 56 sitting *inside* the claim,
and it misses the other route: the sweep does not stop at the advertised count, it keeps writing
until `ISO15693_POLLER_WIPE_ABSENT_RUN` blocks in a row answer nothing. A card silent from block A
is still attempted through A+7, and a write that lands resets the run — so 56 and 57 both go at a
claim of **49**.

Measured against the poller rather than derived:

```
44..48  ->  56 untouched, 57 untouched
49..52  ->  56 ZEROED,    57 ZEROED
```

So the old wording called a card advertising 49–56 safe when the sweep reaches its UID registers —
it understated the hazard on exactly the band it was meant to bound. Two tests now sit on 48 and 49.

That figure is yours, from thread 01, and I took it without checking it. Saying so because you asked
for the same in the other direction. It was invisible until this week: every gen1 card here was the
56-block LRi2K, where 56/57 are the first two blocks past the claim and the question never comes up.

**The correction runs toward safety.** Of the five gen1 cards now measured, only the LRi2K reaches
its own UID registers under a wipe; on the 28- and 40-block ones the run opens at the claim and
trips seven blocks later, nowhere near 56. The armed-gen1 hazard is narrower than this tree has been
saying, and the release notes now say which cards it applies to instead of "most".

## The self-review

`/pr-review-toolkit:review-pr`, seven agents, over the whole diff rather than the delta. It found one
user-facing gap, one contract hole, and about twenty prose defects; the prose ones are on their own
commits.

**The user-facing one.** A wipe that loses the card never said the identity check had not run. It
returns before the verify state is entered, so `uid_verified` stays false; `has_details` sent
`CardLost` to its default arm, so there was no Details button; and "UID not re-checked" is reachable
only through Details. By then the sweep has written 56/57 — which on an armed gen1 card **are** the
UID. So the card can be gone and its identity with it, and the screen said only that it had been
removed. The comment at that branch called the case moot because the card had left; the reason it is
not moot was ten lines below it, in the same comment block.

`has_details` now covers `CardLost` when the mode is wipe and the check never answered. Two things
there were not obvious: the existing note blames a field reset that never happens on this path, so
the wording is chosen by route; and the block list is **suppressed**, because the sweep's own
card-lost check says a lifted card "can surface as a pile of blocks that wouldn't clear". Four
tests, each checked to fail for exactly one reason.

**The contract hole.** The wipe set its live progress denominator — the card's advertised count —
above the geometry guard rather than below it. A card that claims blocks but reports block size 0
returns at that guard, so the measured figure never overwrites it, and `blocks_total` carries the
card's own unverified claim into a terminal event. `blocks_total`'s contract says terminal events
only ever report the measured figure. Nothing renders it today; one statement moved eight lines
down, with a test.

## What the review did not find, which is the more useful half

A full correctness pass over the ISO15693 surface found **no functional defect**. Checked and clean:
every alloc/free pair including the early returns, buffer and index safety against card-supplied
counts, the tick-wraparound form, the tail-drop arithmetic at all five loop exits, all thirteen
reason codes reaching a titled screen with a button set, the three-way right-slot rule agreeing
between `on_enter` and `on_event`, and the Back-swallow argument protocol by protocol. The "widget
copies its string" constraint was checked in the firmware source rather than assumed. Nothing
anywhere treats a refused backdoor write as proof it did not land.

Reached independently, that agrees with where you left round 9.

## The mechanism behind rounds 8, 9 and 10

`tools/gen1-staleness.py` is a scanner I wrote to find the comments the hardware sessions
invalidated, and the gen1 round used it as its work list. **It could not see a single one of the
eight stale sites that survived** — including two consent-screen strings still telling users gen1
was untested.

Those two lost the sentence rather than gaining a corrected one. The caveat was load-bearing only
while gen1 had never been validated at all; what the user consents to is the write and its blast
radius, and how many cards the path has been proved on is a fact about this project rather than
about the card in their hand. The Validation section in the release notes already carries it.

Two gaps, both the same mistake: the patterns match the phrasing that was *already fixed*.
`latch(es)?\b` does not match `latched`, so of the three sites in the tree it flagged the two that
were correct and missed the one that was wrong. And the validation pattern matched "validated" and
never "tested" — the three sites the round removed all said "NOT hardware-validated"; the survivors
all said "not hardware-tested".

A pattern list written by reading the sites you just fixed encodes their vocabulary, and is
systematically blind to the ones you missed. It then reports the job done. It now carries a list of
known-stale phrasings asserted to match, which runs on every invocation and refuses to scan if it
fails, and it exits loudly when given no files rather than printing nothing and returning 0.

That is a better answer to why round 9 found nine defects round 8 introduced than any of the
per-thread ones.

## What is left

Two passes from the plan are still outstanding — **C**, deletion only, verifiable by code bytes
unchanged; and **D**, correct simplification, which rewords live claims and so carries an accuracy
check inside it.

The review also turned up work I did **not** do, because it was outside what I had scoped: four
type-design items (a bitmap write bounded only by its call sites; three `default:` arms that would
absorb a fourteenth reason code silently; the result struct having no mode field though six of its
sixteen are mode-scoped; and the gen1 block numbers and gen2 register refs sharing both a type and a
`BLK` spelling), two implicit narrowings, and two zero-coverage gaps — the reason-code selection
ladder, and the gen1 consent screen, which is the one screen where the wrong button destroys a
card.

**How much of that do you want inside this PR?** My read is that the two test gaps and the enum
exhaustiveness are worth it and the rest is not, but it is your diff to carry. The mechanical
argument for doing it here rather than later is unchanged: this squashes, so tidying inside it costs
`dev` nothing, while the same work afterwards is a second PR and a diff of pure comment churn
against a released app.

One correction to something I told you earlier: the twelve `widget_add_string_multiline_element`
calls can be wrapped after all. The objection I recorded — that the per-site y values carry the line
budget — no longer holds, because that rationale is already hoisted to the file's header. I declined
it on diff cost rather than on principle: twelve branch bodies touched, in the file that has
absorbed the most churn, to save about twenty-five lines of formatting.

Both firmwares warning-free, 114 host tests, format clean.
~~~~
