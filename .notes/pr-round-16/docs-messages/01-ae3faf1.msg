NFC Magic ISO15693: a short 2.3 entry, and ISO15693.md as the reference

The 2.3 entry had grown to 165 lines, about half the changelog, and most of it
described how the ISO15693 support behaves and what it was tested on rather
than what the release changed. Later fixes, and gen3 support if it comes, would
have had to correct it in place.

It is now a short entry: what 2.3 adds; that the scan shows every ISO15693 tag
as a Magic candidate, since magic status is only confirmed by writing; the
three things a user needs before a wipe -- a gen3 card can be bricked, a gen1
card's UID can move, and only one tag should be in the field at a time; and a
pointer to the reference.

The rest is in ISO15693.md beside the changelog, laid out as a reference to be
kept current rather than as release notes: the magic types, what each operation
does, what each result screen means, the limits, and what it was tested on. It
also covers a few things the old entry never said: Info on a card that does not
report its memory, the screen for a file of the wrong type, that block locks
are not copied, and what a file whose blocks are not 4 bytes does to a card.

Documentation only; no code changes.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
