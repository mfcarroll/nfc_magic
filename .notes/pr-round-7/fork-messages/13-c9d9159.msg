NFC Magic ISO15693: revert the cut-at-the-claim operator, and pin it

Round 7, thread 3948189021. You ask for the `<` -> `<=` we made for you in
round 6 (ddd47988) to be reverted, and you are right.

blocks_advertised is a COUNT; cut_block is an INDEX. At equality the claimed
blocks are 0..N-1 and the cut sits at index N -- the first block PAST the claim.
So "at block N of the N this card claims" names an index that is not among the
N, and reads as a completed fraction on the one boundary where the sweep really
was cut. The wording you originally sent us away from was the accurate one.

Your three-line justification at :89-91 is dropped as you asked. It was a
non-sequitur: "block N itself was not attempted" is equally true of both
branches, so it cannot pick between them. Replaced with the count-vs-index
reasoning, which can.

AND IT IS NOW PINNED. This boundary has been got wrong twice, once by each side
of the review, and nothing tested it -- every existing case sat strictly below
the claim (cut 23 of 70, 55 of 64, 10 of 256). test_cut_at_the_claim_reads_as_
past_it asserts that cut == advertised renders "past the 64 this card claims"
and NOT "of the 64", and that one block lower still reads as inside the claim,
so the boundary is the only thing moving. A third round-trip on this line is now
a red test rather than a review comment.

Host tests 106 -> 107. Mutation-verified: restoring `<=` fails the new case.

