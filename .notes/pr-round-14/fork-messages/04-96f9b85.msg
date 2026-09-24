NFC Magic ISO15693: the sweep's attempted-count invariant, stated once

"`block` is the number of blocks attempted" is what every piece of arithmetic
after the sweep depends on, and it was stated three times in three vocabularies,
none naming it as an invariant -- "keep `block` the attempted count, as every
other exit does", "attempted, so it counts -- see the note at the other card-lost
exit", and "count this block into the tail arithmetic below".

The cross-reference in the second is the tell: a rule that needs a see-also to
another exit wants one home.

Hoisted to the loop head, which has room for the two things no site carried. The
deadline check is the one break that does NOT increment, because it runs before
its block is attempted -- that asymmetry is why it needs stating once rather than
per exit. And the invariant is the WIPE's: this sweep attempts every index it
reaches, while the clone skips the four backdoor registers on a gen1 run and so
has a cursor that runs ahead of its count. Stated flatly, without that contrast,
it would be false for the other loop.

Comment-only; the three block++ statements are untouched.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
