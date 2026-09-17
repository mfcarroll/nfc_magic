# Round 13 — main reply (DRAFT, NOT POSTED, and NOT to be posted without mfcarroll)

Prepared while he was away. **Nothing here has been reviewed by him**, which is why it is a draft
and not a push.

Post between the `~~~~` markers, and only after that read.

~~~~
## Round 12 addressed, and the simplification pass with it

All seven are fixed. The two you put first — the gen3 hazard text and the "not memory" fact — are
the two I would have wanted flagged, and both were the trim taking something it should not have.

The gen3 bullet is the one worth dwelling on, and not for the claim. The opt-in writes 56/57/62/63,
a gen3 card keeps its UID at 0x10/0x11 and its signature at 0x14/0x15, and on a card that big those
four are ordinary user data — so it is damage from the opt-in and brick from the wipe, which is
where it started. What made it bad was that the same edit deleted the only record of the addresses
that falsify it: unfalsifiable and wrong arrived together. That is a failure mode to watch for in any
trim, and it is now written down rather than just fixed.

**The reach rule is a closed form, stated once in the poller**, and the release notes carry only the
qualitative half. I checked your derivation against the code rather than reading it: both gates must
pass, `L = max(A + 7, claim - 1)`, and 56 is reached from `A >= 49` **or** `claim >= 57`. Your second
point is the one I would have missed — a landing *read* resets the run too, so it can have been reset
inside the `wiped == 0` branch. Four rounds of paraphrasing something the code states exactly is the
argument for the form you proposed, not for a better sentence.

**Three of the seven were one defect**: a claim corrected in one place and left standing in its twin
— the consent screen fixed and the release notes not, the in-band error code scoped at its definition
and not at its two users. A file that contradicts itself is worse than one uniformly wrong, because
the reader has to work out which half is current. They are fixed together.

## The simplification pass, in the same push

P1-P3 from the self-review — item 2 on the list. P4 stays declined on diff cost: the wrapper for the
twelve `widget_add_string_multiline_element` calls works, and the per-site `y` rationale is already
in the file header so it would strand nothing, but it touches all twelve branch bodies for 25 lines
of formatting.

All three were the same shape as the comment findings above: one rule spelled in two places, each
copy carrying part of the reason.

- **The clock cut**, written out in the clone's loop and the wipe's sweep. One copy said why the cut
  is recorded on the instance, the other why the comparison is elapsed-against-budget and asked
  before the block is attempted. `pass_truncated` and `pass_cut_block` gate Retry in seven places and
  now have one write point.
- **The succeeded-blocks figure**, subtracted on three lines with the reason 600 lines away.
- **The block-size clamp**, once against the macro and once against `sizeof` a buffer declared from
  it. They agreed by coincidence.

These are the first code changes since the comment cut, so none of them has the byte-identical
safety net. Each landed with a test in the host harness — not part of this PR — and each was
mutation-checked from the committed baseline rather than asserted: an absolute deadline passes every
other test in the suite and fails only the new wraparound case; dropping the saturation fails both
inverted inputs; removing the clamp fails both over-large ones. 120 tests, both firmwares clean.

**This should still not merge**, unchanged: the unaddressed WRITE BLOCK limit. Addressed writes and
the gen1 caveat gate are next, and the scoping call on whether they belong inside this PR is yours.
~~~~
