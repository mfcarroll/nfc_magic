NFC Magic ISO15693: drop the UID claim the poller retracted, from the screen that kept it

The NothingWiped branch asserted "The UID was never touched", which is the exact
inference e71dce03 added iso15693_poller.c:1244-1252 to refuse: the sweep sends
three WRITE BLOCKs each at 56 and 57 before giving up, and a tag can apply a write
without answering, so on an armed gen1 card this path CAN move the UID. Both
statements were in the tree at once and only one of them can be right.

The tell is that this comment block was edited in round 7 without the
contradiction being noticed -- the cut shortened prose without re-reading what sat
two lines away from it.

The second half was independently wrong. "-- say so, since the generic 'not a
magic tag / UID write' message would be wrong for a wipe" describes a screen that
does not exist: neither rendered string mentions the UID at all. They are "No
block accepted the / zero-write. / Timed out at block %u." and "No blocks could
be / cleared -- the card / accepted no zero-write."

So there is no behaviour to correct. Something once intended this screen to state
the UID was safe; it never did, and the poller's position is now that it could not
have said so honestly. Deleting the sentence is the whole fix.

What is left is why a cut wipe gets reported on this screen rather than on
WipeStopped. Three lines out, the two fragments the deletion left rejoined. Net -2
comment lines, no code.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
