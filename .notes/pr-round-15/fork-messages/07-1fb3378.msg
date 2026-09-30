NFC Magic ISO15693: the gen1 registers, addressed and no longer mis-scoped

Two changes to the same four addresses -- who a write there can reach, and who a
wipe there can hurt. They are one region of the card and they arrive together.

== THE SEQUENCE CARRIES THE CARD'S ADDRESS ==

The four gen1 backdoor frames now go out as addressed WRITE BLOCKs, flags 0x22
with the UID in the frame, sent through the same helper as the data blocks.

These were the last standard writes in the feature to go out unaddressed, and
the worst to leave open. They are ordinary WRITE BLOCKs aimed at blocks
56/57/62/63 -- on any tag large enough to have them, that is user data -- and
they go out behind an opt-in whose warning is about the card in the user's hand,
not a second one in the field. #251 asks for exactly this.

ADDRESSING THEM IS ALSO WHAT MAKES THE CARD ANSWER. At block 62, an unaddressed
write gets silence from NXP ICODE SLIX and SLIX-S, while the identical addressed
write gets a refusal that parses -- error 0x0F -- and a UID one byte wrong gets
silence again, bracketed by a reader either side to show the card was there
throughout. ST LRi2K answers both forms, with the specific 0x10, "block not
available". Three chips, five cards, and on four of them the sequence had been
going out in the one form the card ignores.

That is also the answer to "is addressing these safe" -- it was already being
done. The wipe's sweep zeroes 56/57/62/63 through the addressed path, and the
clone's conversion path writes 56/57 addressed and watches the UID follow, which
is how a gen1 card on the gen2 path is detected at all.

THE UID REGISTER ITSELF FILTERS ON THE ADDRESS. A write to block 56 with the
address one byte wrong, carrying data the register does not hold, is refused and
the UID is unchanged when read back -- one card of each gen1 chip. Block 56 is
the register the whole sequence exists to keep off a bystander, so the address
filter is measured on the frame that matters most, not only on the data blocks.

THE ONE THING THE SEQUENCE COSTS is a re-address in the middle. Block 56 carries
uid[7..4] and takes effect immediately -- no power-cycle. So by the time block
57 goes out the card has stopped answering to the address the frame before it
used. Without the re-address, 57 is sent to a card that is no longer there and
the run ends with half a UID written: neither the identity the card had nor the
one asked for. The unaddressed form had no such seam.

AND IF THE RE-ADDRESS MISSES, THE VERIFY SAYS WHERE THE CARD WENT. An inventory
that misses, or that a second tag in the field answers, leaves the address where
it was, and 57 then goes to a card that has left it: half a UID after all. A 56
frame lost outright leaves the other half. The gen1 verify took any UID but the
target as "not a gen1 card". One of the two UIDs a half can leave is now
reported the way the gen2 verify reports one: the card is gen1, and the UID it
answers to is the only way to find it again. Only those two. Any other answer
did not come from these frames, most likely a second tag answering the 1-slot
inventory, and it keeps the gen1-failed screen, which names the four blocks the
user's consent has just cost.

The re-address is not new machinery. iso15693_poller_predict_uid and
iso15693_poller_readdress are the sweep's, and the prediction is the check: the
only UID accepted as the new address is the one this write implies. A card
answering anything else keeps the old address and the sequence carries on --
which on a card that is not gen1 is the ordinary case, since there the UID never
moves. The verify's two UIDs are predicted the same way.

UNLOCK AND COMMIT ARE ADDRESSED TOO, and their addressed behaviour cannot be
validated. No gen1 card here has ever accepted either frame, in any form,
including one this app had never written -- so there is no acceptance to observe
and no way to show the addressed form fares differently. They are addressed on
the safety argument alone. What the same evidence shows is that none of the five
cards here needed them: each took the write to block 56 with no unlock or commit
in front of it. Their histories are unknown, so that is a statement about these
cards and not about gen1. They stay -- proxmark sends them, and the cards that
would need them are ones nobody here has -- but the comment at their definition
no longer claims a locked state before them.

BENCHED on all three gen1 chips. A Write UID to a target differing from each
card's own UID in BOTH HALVES, plain Success on each and the whole UID read
back -- not the half that a broken re-address leaves, which is why the target
was chosen that way: block 56 carries the printed tail and 57 the printed head,
so a run that wrote 56 and lost 57 would have left the card's own head behind.
DSFID survived per card.

The frames go through write_block_addressed, the data blocks' own sender,
because a gen1 backdoor write IS a WRITE BLOCK: the block number is what makes
it magic, not the command. The response is read for one thing, whether the card
asked for the OPTION flag, and the result stays discarded -- the 62/63 refusals
are expected and the UID moves regardless, so acting on them would abort a run
that worked.

HEADER NOTES THIS MAKES FALSE. The all-zeros UID note said the card stayed
reachable because gen1's frames carry no UID. They carry it now; the card stayed
reachable because it still answered inventory, and the note says that. And the
unexpected-UID notes, at the result field, at the Fail event and at the reason,
listed the routes to that report without this one; they list it now.

== THE WIPE HAZARD IS NOT LIMITED TO A CARD SOMEONE ARMED ==

The comments scoped the wipe's identity risk to "a card left ARMED by an earlier
gen1 UID write": in the wipe's own notes, at the UID-changed reason and at its
screen. The release notes carry the same correction, in the commit that holds
all of them.

Five cards here take a write to block 56 with NO unlock or commit in front of it
at all, one of them a card this app had never written, and neither frame has
ever been accepted by a gen1 card. Nobody can tell whether a card has been
armed, so the condition a warning can use is not the card's history but gen1
plus a claim high enough for the sweep to reach 56/57 -- the reach rule, already
stated once in the poller.

It matters because it UNDERSTATED the hazard, and in the direction that costs a
user a card: nobody can know whether a tag was written before they got it, and
nothing can detect it, so a warning aimed at that condition asks them to worry
only if they know something unknowable -- while every gen1 card measured behaves
the same way.

THE REGISTER NAMES ARE LABELS, and now say so. UNLOCK and COMMIT read a purpose
into the order proxmark sends 62 and 63 in, and no gen1 card tested has accepted
either. The define says the names are speculation, the comments that stated what
62/63 hold describe the gen1 sequence's bytes instead, and the one screen that
named them -- the gen1 note in Details -- now reads "56/57/62/63 are the UID /
backdoor registers, not file data".

Nothing is claimed in the other direction. There may well be a lock state; an
acceptance of unlock or commit would have proved one, and a refusal proves
nothing, since a card locked or unlocked before it arrived is indistinguishable
from one that never was. The frames stay. What comes out is a precondition
asserted on the hazard. The measurements keep what they measured; only the
inferred state comes off them.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
