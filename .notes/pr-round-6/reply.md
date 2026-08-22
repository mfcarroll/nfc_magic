# Round 6 reply — DRAFT, not posted

---

Thank you for tracing the three rather than taking the report for them, and for the thirteen-row button
table — that is the shape of check I could not run on myself.

Everything is fixed. One thing I want to put at the top rather than bury, because it is the argument you
were making and my own numbers now make it better than the percentage did.

## The comment ratio got worse, and that is the finding

Correcting eleven false comments cost **+117 comment lines against +28 code**, and
`iso15693_poller.c` went from 43% comment to **44%**. Every round that corrects a claim adds the
explanation that makes it correct, so the metric moves the wrong way even when each individual edit is
right.

Which means the file is now in a state where the maintenance burden is the comments themselves. Three of
your eleven threads were comments contradicting *other comments in the same delta*; two were user-facing.
That is not a density problem, it is a **duplication** problem: the same fact stated in three places
drifts in two of them.

So the comment cut is no longer a queued nicety. It is the fix for the class of defect this round was
mostly made of. See the end of this comment for what I would like from you on scoping it.

## Blocking — `iso15693_poller.c:1131`

Fixed, one conjunct, exactly as you wrote it. Your trace is exact including the step I would have got
wrong: `over_capacity` folding in at `:756-760` is what makes `failed_count == blocks_total`, so the guard
trips on a count the back-fill created rather than on anything the card did.

It is the same fabrication as the round-5 blocker, one branch earlier in the same function, and it landed
on the outcome with the most blocks above the cut. `iso15693_poller.h:38` had already promised the
opposite in as many words — "whatever the counts say" — which is the clearest possible statement that the
code and its contract had diverged.

Two tests: the cut case is Partial, and an **uncut** clone that accepted nothing is still Fail, which is
the half of the guard that has to keep working. Mutation-verified — dropping the conjunct fails the first.

## The eleven claims

All corrected. The ones where you changed my understanding rather than just catching a slip:

**The back-fill is clone-only.** I had it as a property of truncation in three places. A cut wipe records
nothing above its cut — the sweep breaks out and both bit-setting sites are inside the body the deadline
gates, so the unreached blocks are outside the denominator rather than inside the numerator. Your
observation that this is *why* `has_details(WipeStopped)` is unconditional is the part I had not joined
up, and it is now the reason given in the header.

**Two cards fused into one.** You are right that the refuses-every-write-answers-every-read card proves
*every* block present, so `blocks_total == cut_block` exactly — and right that above the claim the gap is
bounded by `ABSENT_RUN`, because getting there at all requires never hitting that many absences. The
large gap is the other card entirely. Both are now described, in opposite directions, with the mechanism
for each.

**The cost derivation inverts.** Accepted in full. The probe clause was comparing the clone against its
own earlier revision while the sentence around it compared clone against wipe, and the sweep already
reads on every refused block. So the **wipe** is the more expensive pass on the same geometry — it pays
an inventory every `ABSENT_RUN` absences and a full trailing re-probe, neither of which the clone has.
The clone is dearer only when the source's block size exceeds the target's, and then by the frame ratio,
not the payload ratio. The rule survives; the identification of which pass is dearer was backwards.

**The prefix property was doing two jobs, not three.** The third belongs to `pvPortMalloc`'s memset, as
the read-back note itself says. Two independent facts — one says a dropped block *was read*, the other
says an unread entry *reads as empty* — and the keep branch leans on both. Corrected to two plus a
pointer.

**Classic always prompts.** Accepted, and thank you for flagging it against yourself as well; I would not
have found it, since I checked the shared routing when you first raised the parallel and not the Classic
precondition either. `uid_locked` unconditional with no zero-problems shortcut. The claim is now Gen2
alone, in the CHANGELOG and in `file_select.c`.

**The gen3 wipe.** The sharpest of the eleven. The wipe has no magic check at all — menu, confirm, sweep
— so an un-finalized gen3 card gets its UID registers *and* its configuration signature zeroed, and comes
out no longer identifying as re-writable. The entry now says that explicitly. Still declared rather than
handled, and I have kept your framing that the reader it is written for is the person about to wipe one.

**`NothingWiped` and `uid_verified`.** Count corrected to six reachable reason codes, and the one with no
route now carries the reasoning you drew out: `uid_verified` is false there *by construction*, and the
short-circuit's justification is the one inference this file declines to draw anywhere else — the sweep
did send three WRITE BLOCKs each at 56 and 57 before giving up. Filed, not fixed, as you asked: gen1, no
card, and the short-circuit predates this PR.

**`WipeUidChanged` and Retry.** I took your first reading. Re-running a wipe against a card whose identity
has already moved does not obviously help, and on gen1 it is another pass over the very blocks that moved
it — so the reasoning now sits at the branch ordering in `scene_write.c`, where you said a reader would
look for it. The truncation note still reaches Details there; only the button is withheld.

**The right-slot rule is three-way.** `has_details` → Details; else retryable → Exit; else no right button
at all. You are right that the two-way statement is dangerous specifically because of the instruction it
closes with.

Plus: `COUNT_OF` at all three loops, `#253` for the reboot rather than `#252`, `blocks_total` described as
a count rather than an index, and the 155-column line rewrapped. `ReflowComments: false` is exactly why
neither of us saw that one.

## Simplify

**The fourth `block_is_empty` site.** Yes — and inside a function this delta edited, in the hunk directly
below the one that swapped its local array for the shared one. Five call sites now, no hand-rolled zero
loops left in the file. I will not pretend to be surprised twice.

**The bitmap pair.** Done as a pair, for your reason rather than the line count: both clears sit inside
the densest reasoning in the file, three and five lines from a set, where the two operators differ by two
characters.

**The dead `+ over_capacity` term** and the tally it was added to are both gone;
`nfc_magic_partial_details_any_index` sits beside `append_indices` and takes the same range, so "asked
over exactly the range that will be printed" is structural instead of a comment.

**The render table.** We converged. I had built the `switch(reason)` version, measured it at +25 code /
+21 comment, and stopped — for the reason you give: seven of twelve bodies are dynamic and `wipe_stopped`
arriving dynamic moved the ratio further away. Your `{reason -> title}` is the version that survives
contact, and it is in: one lookup, one hoisted call, twelve identical calls gone. It comes out at
+12 comment / +8 code, which I am reporting rather than dressing up — the lookup is longer than the calls
it deletes, and what it buys is the invariant stated once.

## Verification

- Momentum 87.15 and Unleashed 88.2, both clean from scratch, zero warnings, `clang-format` clean.
- Host tests **101**, all green — up from 83. Two new files this cycle cover the result screens and the
  write scene's routing against GUI recorders, and the round-5 button bug is now expressed as an
  invariant over all thirteen reason codes rather than as a case.
- Commit hygiene: **zero** intra-batch churn across the nine commits.
- Everything here is source-level. Nothing in this round is hardware-verifiable without the lowered-budget
  build, and the six truncation screens I rendered that way last cycle are unchanged by it.

## What I would like from you

The comment cut is the last item on the deferred queue and, on this round's evidence, the one that
matters. Before I spend a round on it I would rather agree the scope, because it is the pass most likely
to move lines under you:

1. **Ownership, as originally sketched** — the event enum owns the outcome contract, the
   `ISO15693_MAGIC_BLK_*` defines own the wire facts, `gen1_optin.c`'s strings own the user-facing gen1
   consequence, `ISO15693_POLLER_PASS_MAX_MS` owns the budget rationale, and everything else
   cross-references instead of restating. That directly targets the three-places-drifts-in-two failure.
2. **Or narrower** — only the facts that have already drifted twice: the back-fill's scope, the two
   truncation archetypes, the prefix property, the cost figures.

I lean towards (1), because (2) leaves the mechanism that produced this round intact. Either way I would
rather do it as its own delta with nothing else in it, so the diff is legible as one decision.

And the compact-UID formatter's four copies go in that pass, per your note.
