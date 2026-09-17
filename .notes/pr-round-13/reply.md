# Round 13 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers.

**No internal process in the payload.** He reads the delta.

~~~~
## Round 12 addressed, the simplification pass, and three things the pass itself turned up

All seven of round 12 are fixed. The two you put first were both the trim taking something it should
not have, and the gen3 one is worth dwelling on for its shape rather than its content: the same edit
that made the claim stronger deleted the only record of the addresses that falsify it. Unfalsifiable
and wrong arrived together. Both sets are now named rather than referred to by position, since
16/17/20/21 are numerically *below* the four the opt-in writes.

One clause did **not** come back with the "not memory" fact. The original sentence carried "so a
card's advertised block count is its capacity", which contradicts the premise of the sweep, the
36-of-64 measurement recorded beside it, the clone's reason for not capping at the claim, and
`blocks_advertised` in the header — all of which exist because the claim and what a card holds
differ. Neither thing that depends on the fact needs that clause.

**The reach rule is a closed form, stated once in the poller**, with the release notes carrying only
the qualitative half. Writing it out found more than your comment did: there is a **third** gate, the
run has to survive the re-probe, and that is what fixes the definition of A. It is the first block of
the FINAL unbroken run, not the first silence anywhere — your point about a landing read is the
load-bearing half of that. A card reading 0-4, silent at 5, reading 6-59 and silent from 60 does
reach 56/57, and "first silence" says it does not. Both errors ran in the under-warning direction, on
the one path where the UID check never runs.

**Three of the seven were one defect**: a claim corrected in one place and left in its twin. A file
that contradicts itself is worse than one uniformly wrong. They are fixed together.

## The simplification pass

P1-P3 from the self-review, and each turned out to be the same shape as the findings above — one
rule spelled in two places, each copy carrying part of the reason.

- **The clock cut**, written out in the clone's loop and the wipe's sweep. Two fields that gate Retry
  in seven places now have one write point. The budget stays a parameter rather than folded in,
  because #253 contemplates a longer one for the clone alone.
- **The succeeded-blocks figure**, subtracted on three lines with the reason 600 lines away. Worth
  noting `blocks_total` has three forms, not the one the first draft of the note described: measured
  on a wipe, backdoor-deducted on a gen1 clone, the source's count on a gen2 clone.
- **The block-size clamp**, once against the macro and once against `sizeof` a buffer declared from
  it. The two callers are opposites rather than one case — the clone's value arrives 1..255 from a
  loaded `.nfc`, the wipe's 1..32 from a five-bit field on the wire, so only one of them can ever
  trip. Saying "neither controls its input" invites deleting one, and the one that looks redundant is
  not the one that is.

**Three further items came out of reviewing that pass**, and they are in the same push:

- the sweep's attempted-count invariant was stated four times in four vocabularies, one of them
  needing a see-also to another exit. Hoisted to the loop head.
- the pass-cut helper returns a bool and writes two fields, one of which is an exclusive-end index
  every screen reports as fact. Its name and doc now say so.
- **this is also the answer to your P4 question.** The twelve `widget_add_string_multiline_element`
  calls do not want a wrapper, and the reason is not diff cost. The duplication is in the
  *documentation* of the layout, not in the calls: the note explained both y values and never the x,
  so the two branches at x=4 read as an inconsistency. They are not — they match
  `nfc_magic_scene_gen2_wipe_partial.c` and `nfc_magic_scene_uscuid_ul_partial.c` exactly, and are
  the two branches whose comments say they mirror those screens. Making all twelve agree would break
  that match to remove an inconsistency that is not one.

These are the first code changes since the comment cut, so none has the byte-identical safety net.
Each is covered in the host harness — not part of this PR — and mutation-checked from the committed
baseline. The reach fixtures were rebuilt too: every one had staged A == claim, the single case where
both terms of the rule collapse into one, which is why four paraphrases could each be wrong somewhere
new without anything going red. 123 tests, both firmwares clean.

**This should still not merge**, unchanged: the unaddressed WRITE BLOCK limit. Addressed writes and
the gen1 caveat gate are next, and the scoping call is yours.
~~~~
