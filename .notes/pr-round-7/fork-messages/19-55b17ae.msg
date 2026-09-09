NFC Magic ISO15693: a gen3 wipe costs the card, and both warnings say so

@0x6r1an0y, who wrote proxmark's ISO15693 V3 magic support, on #255: "Zeroing
blocks 0x14/0x15 on an un-finalized V3 card not only clear the signature, but
also brick the card forever."

Two places said otherwise, so both are fixed here -- one finding, one commit.

THE CHANGELOG described the outcome as "a card with a moved UID that no longer
identifies as re-writable": recoverable-sounding, and the recovery it implies
does not exist. Attributed, not asserted -- no gen3 card exists on either side
of this PR, so nothing here has been observed and the entry says so. The claim
rests on his authority as the person who implemented the V3 path, which is the
strongest source available and still not a measurement.

THE WIPE CONFIRM said nothing at all, which matters more: the CHANGELOG reaches
whoever reads release notes, and this reaches the person holding the card.

    Wipe? (gen1/gen2 only)
    Zeroes every block, including
    gen1 magic 56/57/62/63.
    This can brick a gen3 card!

"brick" is bold via the `\e#` inline marker, which switches to FontPrimary -- a
different font rather than a weight, so it changes the line's width and the wrap
arithmetic with it. Confirmed on device, along with the title: it more than
doubled in length and widget_add_string_element runs off the canvas rather than
truncating, so the closing paren was worth checking.

The title is a scope statement, not a description. "Wipe gen1 / gen2 card?"
would read as the app asserting a generation it cannot detect, and could defuse
the third line for exactly the person at risk; the parenthetical cannot be read
as an adjective on the card.

text_height drops from the default 54 to 38, which is a fix rather than tidying.
At 54 the box runs to y=67, past the 64px screen, so an over-long string would
have drawn UNDER the button instead of clipping -- the three-line budget was
enforced by a comment and nothing else. 38 is y=13..51, stopping just above the
button box. The other branches still use 54 and are not touched: their strings
are not measured.

Deliberately NOT a pre-flight probe. That is #255 and a feature, and the wipe
performs no magic detection at all, so this line cannot say "your card is gen3"
-- only what a gen3 card would cost. A static warning is honest about that; a
borrowed WriteProblems page would not be, since its header reads "Warnings: 1 of
2" and would frame an un-run check as a completed one.

The only gen3 outcome claim left elsewhere is #255's own text, which still
carries the softer wording. Editing an issue means posting, so it is not done
here.

