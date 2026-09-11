NFC Magic ISO15693: reunite write_step's doc comment, which the extraction split in two

9c14213d pasted finish_write between line 1 and line 2 of write_step's two-line
doc comment. So the first half headed finish_write, describing the wrong function
and stopping mid-clause on "Returns the", while the second half sat above
write_step opening with no subject. Neither function was correctly documented.

You are explicit that the extraction itself is right and does what you asked for
-- this is only where the hunk landed. Moved the orphaned line down to rejoin
its other half.

The same paste left a ragged wrap inside finish_write's own comment ("-- so a
change to / the gen2 / arm could"), which 5bc041d4's rewrap pass did not reach.
Rejoined, at 103 columns, which is the width of the three lines around it -- so
this restores the paragraph's original shape rather than setting a new one, and
stays inside the churn-avoidance boundary the cut settled at.

Comment-only, net -1 line.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
