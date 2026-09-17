# Round 12 — main reply (DRAFT, NOT POSTED, and NOT to be posted without mfcarroll)

Prepared while he was away. **Nothing here has been reviewed by him**, which is the reason it is a
draft and not a push: his read has caught more than any other check in this project.

Post between the `~~~~` markers, and only after that read.

**No internal process in the payload** — the same rule round 11 settled. He reads the delta.

~~~~
## The simplification pass — three changes, and the first code since the comment cut

P1-P3 from the self-review, which is item 2 on the list I gave you. P4 stays declined, on diff cost:
a wrapper for the twelve `widget_add_string_multiline_element` calls does work, and the per-site `y`
rationale is already hoisted to the file header so it would strand nothing — but it touches all
twelve branch bodies, for 25 lines of formatting, in the most-churned part of that file. Highest
re-read cost, lowest benefit.

All three turned out to be the same shape: one rule spelled in two places, with each copy carrying
part of the reason and neither carrying all of it.

- **The clock cut**, written out in the clone's write loop and again in the wipe's sweep. The clone
  copy said why the cut is recorded on the instance rather than a local; the wipe copy said why the
  comparison is elapsed-against-budget, which is what survives a tick wraparound, and why it is asked
  before the block is attempted. `pass_truncated` and `pass_cut_block` gate Retry in seven places and
  now have one write point outside the reset.
- **The succeeded-blocks figure.** Three lines on the write-fail screen subtract it and none of them
  said why the subtraction saturates. It is because `blocks_total` and `failed_count` are not two
  halves of one count — the total is what the run measured, or a logical count on a gen1 clone, while
  failures are recorded at true block indices — and they have disagreed before.
- **The block-size clamp**, spelled once against `ISO15693_MAX_BLOCK_SIZE` and once against `sizeof`
  a buffer that happens to be declared from that macro. They agreed by coincidence.

Each is covered in the host harness — not part of this PR — and each was mutation-checked from the
committed baseline rather than asserted: an absolute deadline passes every other test in the suite
and fails only the new wraparound case; dropping the saturation fails both inverted inputs; removing
the clamp fails both over-large ones. 120 tests, both firmwares clean, `ufbt format` clean.

**This should still not merge**, unchanged and for the same reason: the unaddressed WRITE BLOCK
limit. Addressed writes and the gen1 caveat gate are what is next, and the scoping call on whether
they belong inside this PR is yours.
~~~~

---

**SUPERSEDED by [../pr-round-13/reply.md](../pr-round-13/reply.md).** His round 12 arrived before
this went out, so the simplification pass and the round-12 fixes became one round. The fork messages
here for P1-P3 are carried over into round 13 unchanged; this reply is not.
