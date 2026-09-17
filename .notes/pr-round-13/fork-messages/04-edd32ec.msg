NFC Magic ISO15693: the sweep's attempted-count invariant, stated once

"`block` is the number of blocks attempted" is what every piece of arithmetic
after the sweep depends on, and it was stated four times in four vocabularies,
none of which named it as an invariant -- "keep `block` the attempted count, as
every other exit does", "attempted, so it counts -- see the note at the other
card-lost exit", "count this block into the tail arithmetic below", and "the
exclusive end of the attempted range" in a different function.

The cross-reference in the second is the tell: a rule that needs a see-also to
another exit wants one home. A reader reasoning about `block - absent_run` in the
tail-drop had to collect three fragments to be sure of it.

Hoisted to the loop head, which also has room for the part no site carried: the
deadline check is the one break that does NOT increment, because it runs before
its block is attempted. The three site comments are now the same four words.

Comment-only; the four `block++` statements are untouched.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
