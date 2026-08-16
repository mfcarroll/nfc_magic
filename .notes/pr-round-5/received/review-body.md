Apologies for the wait on this one - the review was finished days ago and I simply failed to post it.
That's on me, not on anything in the branch. @xMasterX, thanks for the nudge.

@mfcarroll on the tags: worth having, but nothing here waits on them. None of the three blocking
items below needs a gen1 card - all are reachable on the gen2 card you already have. What gen1 would
settle is the set of things the code currently parks as open on exactly that grounds: the wipe
zeroing 56/57 on an already-armed card, the unlock/commit reading inferred from proxmark's send
order, and the UID re-read that can report a change but not prevent one. Those are arguments today
and would become measurements. Your call whether that's worth holding a merge for; I'd say no.

## Round 5 - `a3e13a3a..f8eb8164`, 13 commits

Builds clean at API 88.0, no warnings, `ufbt format` clean, no tracked artifacts, 149,248 bytes.
Commits are one decision each, as before.

Every item I raised last round is addressed, and two of them are addressed better than I proposed.
Three new blocking items, all introduced by this delta.

### Two things you were right about

**Your correction to my sketch is right and I was one step short.** A kept block has to move
`highest_present`, or `failed_count` exceeds `blocks_total` and the partial screen renders
"Wiped 0/20, not cleared: 44". That follows from the argument I gave and I did not carry it
through. Thank you for catching it rather than implementing what I wrote.

**Partial for a cut sweep is the better call than the doc footnote I asked for**, and the line you
drew is the right one - the operation's own job left undone versus a best-effort check that did not
reach an answer. That the contract got *shorter* is the tell you say it is. I had proposed
documenting a divergence that should not have existed.

### Blocking

**1. On the new "Wipe stopped" screen the button labelled Exit opens Details.** `WipeStopped` is the
first reason code in both `is_retryable` and `has_details`, and the two call sites branch in
opposite orders - `on_enter` tests `is_retryable` first and renders Retry/Exit, `on_event` tests
`has_details` first and pushes the details scene. It also means the `case WipeStopped: return true`
arm you wrote so the truncation note would be reachable never renders a button, so that note's only
labelled route on that screen is a button that promises to quit. This is the drift `b8687ed6`
removed, reappearing one layer up: the old code was correct only because the two sets were disjoint,
and nothing said so.

**2. A clone the clock cuts tells the user their card refused blocks nothing was sent to.**
`pass_truncated` never leaves `iso15693_poller_write_source_blocks`. A clone cut at block 10 of 256
renders "Cloned 10/256 / Not written: 246", lists all 246 indices in Details under **"Blocks not
written"**, and offers **Finish** - not Retry, which is the correct next action. You applied exactly
this reasoning to the capacity claim in `f8eb8164` ("those blocks refused nothing, they were never
attempted") and stopped at the classifier. The same sentence indicts the report.

**3. `CHANGELOG.md:41-43` claims the app reports a fault it actually hides.** For the most likely
shape of a dead stretch - blocks already dead when the card was presented, so activation's read
stopped there - the discriminator cannot fire, the run drops, `failed_count` is 0, and it reports
Success. The `.c` is honest about this at `:985-986`; the changelog asserts unqualified what the code
marks open. One line. I am not asking you to close the code gap - see below for why I think you
can't. `:46-48` has a second, separate error: it names "were read" as the discriminator when the
code tests *non-zero content*, which is not the same claim and is the difference your own helper doc
gets right.

### Worth taking in the same pass

- The tail-drop's keep branch and `wipe_note_present` now state opposite rules about the same
  blocks. What makes the divergence harmless is a fact neither of us had written down - see the
  thread; it belongs in that comment, because without it this reads exactly like the round-4 bug.
- `!pass_truncated` suppresses "Card too small" on the card most likely to spend the budget: the
  clone's cost is dominated by *failing* blocks, and a genuinely too-small card is a long run of them.
- "Stopped at N" prints `blocks_total`, which is the highest block that answered, not the cut. Two
  screens make a false claim about which blocks were attempted, and on the card the bound was
  designed for it renders "Stopped at 200 of 64".
- `uid_verified` is read on one of the three wipe outcome screens. This is the shape of the
  `sweep_truncated` item from last round, recurring with the new field.

### Settled by disassembly, so you don't re-verify

Your blocking fix rests on two firmware facts your bench cannot reach, and both hold:

- `pvPortMalloc` tail-calls `memset(p, 0, size)`. Flipper's `malloc` zeroes, and `simple_array_init`
  does a bare `malloc` with no per-element init for a byte array - so the activation cache really is
  zeros for unread blocks, and the fake-flash card drops its phantoms deterministically rather than
  by whatever the heap happened to hold. Your bench pass could not have distinguished those.
- `iso15693_3_poller_read_blocks` (`0x08044738`) returns at the first failing block, so the cache is
  a **prefix**. That is the fact that bounds the keep-branch divergence above, and it is also why
  finding 3 cannot be fixed in code.

Also confirmed: the activation filter is exactly errors 6 and 9 = `Timeout` and `NotSupported`; the
100ms delay, the 11px pitch, the queue length and the `FuriWaitForever` send are all as you state.
Every firmware figure in this delta checks out. The only enumeration that does not is the gen4
omission in the rewritten Back comment.

### Your two questions

**The clone having no up-front confirm: I agree, and the table settles it.** Consent deferred to the
moment a destructive path becomes real, naming the actual consequence, is better than a fixed warning
whose text describes a hazard gen2 ISO15693 does not have. Your argument against yourself - wipe
prompts, clone doesn't - is answered by the one you gave: a wipe's only product is destruction.

**The test harness is not mine to decide.** It reads as the right thing and the three firmware
semantics you read out of the source are the valuable part regardless. Whether `base_pack` grows a
test directory is @xMasterX's call and @mishamyte's; I've flagged it to them rather than answer for
them. It is correctly out of this PR either way.

The deferred queue is unchanged and I have not re-raised any of it.
