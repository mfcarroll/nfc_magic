NFC Magic ISO15693: the gen1 backdoor sequence carries the card's address

The four gen1 backdoor frames now go out as addressed WRITE BLOCKs, flags 0x22
with the UID in the frame, built by the same helper as every other write here.

These were the last frames in the feature a bystander could take, and the
worst ones to leave open. They are ordinary WRITE BLOCKs aimed at blocks
56/57/62/63 -- on any tag large enough to have them, that is user data -- and
they go out behind an opt-in whose warning is about the card in the user's
hand, not a second one in the field. #251 asks for exactly this, and the rest
of the round already does it everywhere else; shipping the safety fix with
this frame set exempt would have been the first thing a reviewer asked about.

ADDRESSING THEM ALSO TURNS OUT TO BE WHAT MAKES THE CARD ANSWER. At block 62,
an unaddressed write gets silence from NXP ICODE SLIX and SLIX-S, while the
identical addressed write gets a refusal that parses -- error 0x0F -- and a
UID one byte wrong gets silence again, bracketed by a reader either side to
show the card was there throughout. ST LRi2K answers both forms, with the
specific 0x10, "block not available". Three chips, five cards, and on four of
them the sequence had been going out in the one form the card ignores.

That is also the answer to "is addressing these safe" -- it was already being
done. The wipe's sweep zeroes 56/57/62/63 through the addressed path, and the
clone's conversion path writes 56/57 addressed and watches the UID follow,
which is how a gen1 card on the gen2 path is detected at all.

THE ONE THING IT COSTS is a re-address in the middle. Block 56 carries
uid[7..4] and takes effect immediately -- no power-cycle, measured on NXP
ICODE SLIX and ST LRi2K -- so by the time block 57 goes out the card has
stopped answering to the address the frame before it used. Without the
re-address, 57 is sent to a card that is no longer there and the run ends with
half a UID written: neither the identity the card had nor the one asked for.
The unaddressed form had no such seam.

It is not new machinery. iso15693_poller_predict_uid and
iso15693_poller_readdress are the sweep's, and the prediction is the check:
the only UID accepted as the new address is the one this write implies. A card
answering anything else keeps the old address and the sequence carries on --
which on a card that is not gen1 is the ordinary case, since there the UID
never moves.

UNLOCK AND COMMIT ARE ADDRESSED TOO, and their addressed behaviour cannot be
validated. No card here has ever accepted either frame, in any form, including
one this app had never written -- so there is no acceptance to observe and no
way to show the addressed form fares differently. They are addressed on the
safety argument alone. What the same evidence does settle is that they are not
required: five cards take a bare write to block 56 with no unlock in front of
it, and 0x10 means "block not available" rather than "refused", which reads as
those two addresses not existing on this silicon. They stay regardless --
proxmark sends them, and the cards that would need them are ones nobody here
has -- but the comment at their definition no longer claims a locked state
before them.

The frame builder is the standard one now, because a gen1 backdoor write IS a
WRITE BLOCK: the address is what makes it magic, not the command. The response
is read for one thing, whether the card asked for the OPTION flag, and the
send's return stays discarded -- the 62/63 refusals are expected and the UID
moves regardless, so acting on them would abort a run that worked.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
