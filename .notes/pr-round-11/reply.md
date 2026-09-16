# Round 11 — main reply (DRAFT, not posted)

Post between the `~~~~` markers.

**No internal process in the payload.** He reads the delta between the round he reviewed and this
one. When something was cut, which commit in this push removed it, and anything we broke and fixed
before he saw it are all dev bookkeeping.

~~~~
## Round 11 addressed, the no-latch result extended to three chips, and a correction you are owed

All seven threads are fixed; replies are on each.

The one worth the most is `iso15693_poller.h:83`. You were right that the three-chip result cannot
carry a no-latch claim, since set, read-back and restore all run through a power-cycle. Rather than
narrow the sentence, the measurement has been run on the other two chips: one card each of NXP ICODE
SLIX-S at IC ref 0x02 and NXP ICODE SLIX at IC ref 0x01, same frames and same UID as the LRi2K, with
the field held up. All three return the newly written UID with no power-cycle, and both new cards
restored byte-identically. The immediacy result now rests on the same sample as the sentence beside
it, and the header says "the gen1 chips tested". Three chips, not the family.

One claim deliberately did **not** widen. The LRi2K refuses 62/63 in band, error 0x10; on the other
two the client reported a plain command failure, which does not distinguish an error frame from
silence. The refusal replicated everywhere, the in-band evidence is one chip, and the comment keeps
those apart.

The run also settled something this PR had asserted from the other direction. A 40-block card
accepted writes at 56/57 and its UID moved, so the advertised geometry says nothing about whether
the backdoor registers exist — they are registers, not memory, which is what the code already says.

## The comment cut, and the numbers I gave you for it

Both halves are here: the source, and the 2.3 release-notes section from 177 lines to 115.

**The figures in my last reply are wrong and I am correcting them.** I quoted "1,576 comment lines
on this surface, 787 of them in 43 blocks of ten lines or more, and roughly 250–370 removable". On
the tree you actually had, those figures are **1,544 / 740 / 40**, and the defensible estimate was
**~240–350**. Close enough that nothing turned on them, which is why it is worth saying: they were
offered as measurements.

**What came out is well short of that: 84 comment lines, with code `+0 −0`.** Same surface, 1,544 to
1,460. Block mass moved further than the line count — 740 lines in blocks of ten or more down to
610, and 40 such blocks down to 36 — because most of what left a block came back as a shorter
sentence that is still a comment.

The gap is the estimate being wrong, not a target missed. The rule was to cut argument and keep
constraints, and applied honestly, most block content turned out to be constraint: the reasoning a
later reader needs in order not to undo the thing. Hitting 250–370 meant taking the comments that
are load-bearing, so the smaller number is the one I would rather report.

A review pass over the finished cut moved it from 99 to 84, putting back seven places where a
constraint had gone out as though it were an argument — and four sentences that had lost the word
holding them up, one of which inverted its claim. Those are in the round.

The rest belongs with the simplification pass, where fewer fields and fewer branches remove the
*need* for some of the contract rather than the text of it. `iso15693_poller.h` is the clearest
case: still 64% comment, but only about a third of that sits in a block at all — the remainder is
per-field contract, and the only way to shorten it is to have fewer fields.

## Where this stands

Unchanged: **this should still not merge.** The unaddressed WRITE BLOCK limit is the reason.
Addressed writes and the gen1 caveat gate are next after the simplification pass, then a device
re-test including the TI cards.
~~~~
