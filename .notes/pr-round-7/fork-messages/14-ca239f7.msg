NFC Magic ISO15693: four claims the comment cut shortened unchecked

Round 7 threads 3948188925, 3948188962, 3948188996, 3948188946. All four are
comments the round-7 cut touched -- it compressed or re-placed each one and left
the claim inside it wrong. Worth saying plainly, because it is the pattern rather
than four coincidences: the cut treated a comment as a unit to SHORTEN, not a
claim to CHECK, so compression preserved truth-value. Two of these ended up
NEARER the text that contradicts them.

poller.c:41 -- the COUNT_OF rationale was inverted, in the comment whose entire
job is to stop someone reverting the refactor. COUNT_OF is sizeof(x)/sizeof(x[0]);
widen the element type to uint16_t and sizeof goes 4 -> 8 while the count stays
4, so a sizeof-bounded loop runs EIGHT iterations over a four-element array,
touching indices 4..7 -- FOUR elements past the end. Widening lengthens the
loop, it cannot shorten it.

poller.h:160 -- the framing sentence named the wrong pair of quantities. No card
can "separate" blocks_total from cut_block in opposite directions: blocks_total
is highest_present + 1 and is structurally <= the cut, which the line above says.
What two cards differ in is which side of the ADVERTISED COUNT the cut lands on,
which is what the bullets' own labels say. Your phrasing, adopted.

poller.h:164 -- right bound, wrong reason. "Getting past the claim requires never
hitting that many consecutive absences" is false: below the claim the absent-run
trip falls through to continue rather than ending the sweep -- the advertised-count
floor doing its job -- and one later block that answers zeroes the run. What
actually bounds the gap is the run still open AT the cut: the deadline is tested
at the top of the iteration, so on every path absent_run <= ABSENT_RUN - 1 there,
giving cut_block - blocks_total <= 7. Same bound, and it survives the floor.

write_fail.c:273 -- the one that cost you a false finding. "Short-circuits to
this screen before any of the truncation reporting" is a claim about the screen it
is printed on, and the code four lines down disproves it: pass_truncated prints
"Timed out at block %u" right there. The routing half was correct, so the fix is
to say WHICH reporting is skipped -- the WipeStopped screen -- and that the cut is
not lost. You note we had written the accurate version 90 lines up at :185.

107 host tests unchanged; all four are comment-only.

