NFC Magic ISO15693: the succeeded-blocks figure is one subtraction, for three callers

Three lines on the write-fail screen print how many blocks succeeded, each
spelling (a >= b) ? a - b : 0 with no reason given at any of them.

The note leads with the shape rather than one caller's version of it: the total
and the deduction are counted independently, whichever pair a caller passes.
blocks_total's three forms stay in the header and are pointed at rather than
restated, since a restatement of a three-way contract is what drifts. The
deduction varies too -- the clone passes failed_count plus over_capacity, which
is a third accounting rather than the second one.

Saturating because those are independent accountings and have disagreed: the
tail-drop's keep branch can count a proven-present block as failed while
blocks_total stops at the highest block the card proved, rendering
"Wiped 0/20, not cleared: 44" -- the example that branch carries itself. That is
guarded where it happens; this is a statement about what reaches the user if it
recurs. The counts are unsigned, so an inversion would print near 65535.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
