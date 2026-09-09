NFC Magic ISO15693: a wipe reaches every ISO15693 tag in the field

The previous commit cited #251 from the code, but nothing said it to the person
about to press Wipe. It goes in "Validation (at 2.1)", which is where this release's
known limits already live, beside the gen3 entry that cites #255.

Stated as user-facing consequence rather than frame construction: a second tag
in range gets zeroed with nothing on screen saying it was there, and the
post-wipe inventory can make the "UID changed" screen print the bystander's UID
as though it were the card's. Both halves, because the second is the one that
misleads rather than just destroys -- that screen exists to be the route back to
a card whose identity moved, so a wrong UID there is worse than none.

Scoped honestly: any generation, magic or not, since the wipe does no magic
detection; and it needs two tags in the field at once, which a badge holder or
wallet does. Split out at mishamyte's request, so the entry says so rather than
leaving it looking unnoticed.

Also fixes the #255 draft's "neither in scope here", which did not parse -- it
read as though the fix were out of scope in the issue that tracks it. It now
says what was meant: not asking for either in #250.

