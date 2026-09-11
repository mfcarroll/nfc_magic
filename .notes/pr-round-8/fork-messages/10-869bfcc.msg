NFC Magic ISO15693: reunite write_step's doc comment, which the extraction split in two

9c14213d pasted finish_write between line 1 and line 2 of write_step's two-line
doc comment. So the first half headed finish_write, describing the wrong function
and stopping mid-clause on "Returns the", while the second half sat above
write_step opening with no subject. Neither function was correctly documented.

The extraction itself is right; this is only where the hunk landed. The orphaned
line moves down to rejoin its other half.

The same paste left a ragged wrap inside finish_write's own comment ("-- so a
change to / the gen2 / arm could"), which 5bc041d4's rewrap pass did not reach.
Rejoined at 103 columns, inside the 100-105 the rest of that paragraph runs at, so
it restores the original shape rather than setting a new width.

Comment-only, net -1 line.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
