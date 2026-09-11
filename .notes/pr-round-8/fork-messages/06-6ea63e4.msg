NFC Magic ISO15693: this app does clear the commit register -- the argument needs ordering

"nothing ever clears commit again -- not this app, not proxmark's SetTag15693Uid"
was contradicted two lines up in the same sentence chain, which says this loop
reaches unlock/commit (62/63). It does, and it writes zeros there like everywhere
else, so the clause naming this app was simply false.

You are right that the conclusion survives and only the sentence overreached.
What the argument actually needs is narrower: no gen1 UID-WRITE sequence clears
commit afterwards, so a card left armed by an earlier run is still armed when the
next one starts. The reason a wipe cannot save it is ORDER, not absence -- 56/57
are zeroed while commit still holds 0x6996, so the UID latches before the sweep
gets to 62/63.

Stated that way round, with the sweep's own zeroing of commit named rather than
denied. Net +2 comment lines, and the open question and the #255 pointer are
unchanged.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
