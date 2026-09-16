# Round 11 — main reply (DRAFT, not posted)

Post between the `~~~~` markers.

~~~~
## Round 11 addressed, the latch result extended to three chips, and a correction you are owed

All seven threads are fixed; replies are on each. Two of them needed no fix — the comment cut in
this same push had already removed the cost footnote and the "claim of 49" sentence — and the
replies say so rather than claiming credit for it.

The headline one is `iso15693_poller.h:83`. You were right that the three-chip result cannot carry a
no-latch claim, since set/read-back/restore all runs through a power-cycle. Rather than just narrow
the wording, the measurement has been run on the other two chips: one card each of NXP ICODE SLIX-S
(IC ref 0x02) and NXP ICODE SLIX (IC ref 0x01), same frames and same test UID as the LRi2K, with the
field held up. All three return the newly written UID with no power-cycle. So the immediacy result
now has the same sample as the sentence beside it, and the header says "the gen1 chips tested"
rather than "gen1 silicon". Three chips, not the family.

One claim deliberately did **not** widen. The LRi2K refuses 62/63 in band, error 0x10; on the other
two the client reported a plain command failure, which does not separate an error frame from
silence. The refusal replicated everywhere, the in-band evidence is one chip, and the comment now
keeps those apart instead of letting one ride on the other.

Both cards were restored and confirmed byte-identical. The run also falsified one of our own bench
records, which had reasoned that a 40-block card puts 56/57 out of range and so makes a gen1 attempt
harmless. Blocks 56 and 57 accepted the write and the UID moved — the UID registers are backdoor
registers, not memory, so the advertised geometry says nothing about whether they exist. That is
what this PR already concluded elsewhere; the bench note contradicted it and has been corrected. The
app is unaffected, since a 40-block claim stops the sweep well short of them.

## The comment cut, and the numbers I gave you for it

The cut is in this push, both halves: the source, and the 2.3 release-notes section from 177 lines
to 109.

**The figures in my last reply were measured against a tree that never shipped.** I quoted "1,576
comment lines on this surface, 787 of them in 43 blocks of ten lines or more, and roughly 250–370
removable". The measurement was taken mid-round against a working tree, not against what you had.
On the pushed tree the true figures were **1,544 / 740 / 40**, and the honest estimate range was
**~240–350**. The three numbers are close enough that nothing downstream turned on them, which is
exactly why it is worth saying: they were stated as measurements and they were not measurements of
anything you could open.

**What the cut actually achieved is well short of that estimate: 99 comment lines, with code
`+0 −0`.** Same surface, 1,544 to 1,445. The block mass moved further than the line count — 740
lines in blocks of ten or more down to 595, and 40 such blocks down to 36 — because most of what
came out of a block was replaced by a shorter sentence that is still a comment.

The gap between 240–350 and 99 is not a shortfall against a target, it is the estimate being wrong.
The criterion was to cut argument and keep constraints, and once applied, most block content turned
out to be constraint: the reasoning a later reader needs in order not to undo the thing. Loosening
the criterion to hit the number would have cost the comments that are actually load-bearing. I would
rather report the smaller figure than defend the larger one.

A second pass at this belongs with the simplification round, where fewer fields and fewer branches
remove the *need* for some of the contract rather than the text of it. `iso15693_poller.h` is the
clearest case: still 64% comment, but only about a third of that sits in blocks at all — the rest is
per-field contract, reachable only by having fewer fields.

## One regression in the cut, which your CHANGELOG thread caught indirectly

Pointing the release notes at the poller's wording is right, and the cut had deleted that wording —
the carve-out naming the card that answers reads everywhere and so walks past 56/57 whatever it
claims. What survived was still true but left it to be inferred by negation. Restored, and the
release notes now follow it rather than restating it.

That is the second time in this round that compressing dense text broke something the compression
pass could not see, so the cut got a separate read-through for sense as well as for category. The
other finds were a stranded verb, a dangling antecedent, and a metaphor that turned out to be
inverted on the point that mattered.

## Where this stands

Unchanged from the last reply: **this should still not merge yet.** The unaddressed WRITE BLOCK
limit is the reason, and the addressing work plus the gen1 caveat gate is the next item after the
simplification pass. Then a device re-test including the TI cards.
~~~~
