# Round 14 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers. **No internal process in the payload.**

Answers his two comments covering `a27d187d..749f10e6` (twelve commits, four behavioural).

~~~~
## All twelve taken, three of them pending the bench

Every one of the twelve is in, unrebased and unreworded. Where your wording and mine covered the
same ground I kept yours, including in two places where yours is better than what I had written:

- **`88d757b5`.** I had found the same thing in `bb937361` and resolved it worse — I deleted the
  capacity clause outright. Scoping it to the three gen1 chips and naming the gen2 contrast keeps the
  measurement instead of losing it, and it belongs in that opening block.
- **`0c7a2515`'s `blocks_total` correction.** "whichever of the four registers fall BELOW it, which on
  a source under 57 blocks is none of them" is exact, and I had the loose version.

**The three behavioural ones are taken but not yet benched.** I am back at the cards tomorrow and
will report `0c5a7d69`, `4d20f06e` and `62b60e2b` then. Nothing below claims a result for them.

**The fourth needed no bench, and I checked it here rather than taking it.** `749f10e6` holds up at
every step against the firmware source: `send_frame` ends in
`bit_buffer_copy(rx_buffer, instance->rx_buffer)` after the CRC trim; `instance->rx_buffer` is
`ISO15693_3_POLLER_MAX_BUFFER_SIZE`, 64; `bit_buffer_copy` is `if(buf == other) return;` then
`furi_check(capacity_bytes * 8 >= other->size_bits)`, a halt; and **all five** in-tree callers pass
`instance->rx_buffer` as destination and source, so that check has never been live in the firmware.
Our three raw senders really are the first callers anywhere to hand it a foreign, smaller buffer.

On `0c5a7d69` — the part you flagged as worth attention is the right part. The struct comment
enumerates the two paths that return from `Start` before the send, and that enumeration is correct.
It just presupposes `Start` is entered. I wrote that enumeration and did not see past its own frame
either.

## What I have added on top

Four of these are the simplification pass, which your twelve did not touch:

- **the clock cut**, written out in the clone's loop and the wipe's sweep with half the reason at
  each. One helper, one write point for the two fields that gate Retry in seven places.
- **the block-size clamp** — which your `4d20f06e` turned from two callers into three. The helper
  says why they are not one case: the clone's and the CFG's arrive from a loaded `.nfc`, the wipe's
  off the wire in a five-bit field and cannot fire. Your point that the CFG one matters most, because
  those bytes outlive the run, is the reason it leads.
- **the succeeded-blocks figure**, subtracted on three lines with no reason at any of them. It points
  at `blocks_total`'s contract in the header rather than restating it, since your version of that
  contract is the precise one.
- **the sweep's attempted-count invariant**, stated three times in three vocabularies, one of them
  needing a see-also to another exit. Hoisted to the loop head, with the part no site carried: the
  deadline check is the one break that does not increment.

And one answer to a question left open: the twelve `widget_add_string_multiline_element` calls do not
want a wrapper. The duplication is in *describing* the layout, not in the calls — the note explains
both y values and never the x, so the two bodies at `(4, 20)` read as an inconsistency. That pair
appears in exactly three files in the repo, and the two branches here are the ones mirroring the
other two screens.

## One correction back to you

`5c97e16f` is right in its gates, its derivation and its disjunct framing, and I have built on it
rather than replaced it. Two things are missing, both in the under-warning direction.

There is a **third gate**: the run has to survive the re-probe that follows the trip. Your own text
says `wipe_note_present` zeroes the run from the read path and the re-probe as well as a landed
write — so the mechanism is already there, but "A the first block that answers nothing" cannot hold
beside it. A is the first block of the **final unbroken run**, and the third gate is what makes it
terminal.

It bites on a card that recovers. One **advertising 56** that reads 0-4, goes silent at 5, reads 6-59
and is silent from 60 reaches 56 and 57 — while `A = 5` with `claim = 56` makes both disjuncts false.
The claim has to be named there or the example proves nothing, since at any higher claim the claim
term carries it anyway.

The 256 ceiling is in the form now too. It binds the A term alone.

## A note on the test harness, since your commit found it

`e32e6242` broke a build you cannot see. There is a host-side test harness in this repo — it compiles
the shipped `.c` files verbatim against fakes and runs 123 cases over the sweep, the outcome
function and the result screens — and `furi_crash` was not in its `furi` fake, so the poller stopped
compiling there. One line to fix, and nothing on your side could have shown it.

That is the argument for releasing it, which you asked about several rounds ago. It catches exactly
this class: a change that is correct in the firmware and wrong against a second consumer of the same
source. I will put it up as a separate PR rather than adding it here.

## Where this stands

**Still not ready to merge**, and for the same reason as before: every ISO15693 write this app sends
is unaddressed, and TI Tag-it HF-I Plus refuses unaddressed WRITE BLOCK outright. Addressed writes
and the gen1 caveat gate are the next piece of work, and the scoping call on whether they land inside
this PR is still yours.
~~~~
