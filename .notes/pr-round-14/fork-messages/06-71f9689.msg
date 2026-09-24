NFC Magic ISO15693: the reach rule has a third gate, and A is the run the sweep ends on

Builds on the two-gate derivation rather than replacing it: the gates, the
arithmetic and the disjunct framing are unchanged and correct. Two things were
missing, both in the under-warning direction.

There is a THIRD gate: the run has to SURVIVE the re-probe that follows the trip.
The mechanism was already written down -- wipe_note_present zeroes the run from
the read path and the re-probe as well as from a landed write -- but "A the first
block that answers nothing" cannot stand beside it. A is the first block of the
FINAL unbroken run, and the third gate is what makes it terminal.

It bites on a card that recovers. One ADVERTISING 56 that reads 0-4, goes silent
at 5, reads 6-59 and is silent from 60 reaches 56 and 57, while A = 5 with
claim = 56 makes both disjuncts false. The claim has to be named for that example
to bite -- at any higher claim the claim term carries it regardless.

The 256 ceiling is in the form now as well. It binds the A term alone, since
claim - 1 cannot exceed a block count the wire caps at 256.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
