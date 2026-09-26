NFC Magic ISO15693: the gen1 registers, addressed and no longer mis-scoped

Two changes to the same four addresses -- who a write there can reach, and
who a wipe there can hurt. They are one region of the card and they arrive
together.

== THE SEQUENCE CARRIES THE CARD'S ADDRESS ==

The four gen1 backdoor frames now go out as addressed WRITE BLOCKs, flags 0x22
with the UID in the frame, built by the same helper as every other write here.

These were the last frames in the feature a bystander could take, and the
worst ones to leave open. They are ordinary WRITE BLOCKs aimed at blocks
56/57/62/63 -- on any tag large enough to have them, that is user data -- and
they go out behind an opt-in whose warning is about the card in the user's
hand, not a second one in the field. #251 asks for exactly this, and the rest
of the round already does it everywhere else; shipping the safety fix with
this frame set exempt would have been the first thing a reviewer asked about.

ADDRESSING THEM IS ALSO WHAT MAKES THE CARD ANSWER. At block 62, an unaddressed
write gets silence from NXP ICODE SLIX and SLIX-S, while the identical addressed
write gets a refusal that parses -- error 0x0F -- and a UID one byte wrong gets
silence again, bracketed by a reader either side to show the card was there
throughout. ST LRi2K answers both forms, with the specific 0x10, "block not
available". Three chips, five cards, and on four of them the sequence had been
going out in the one form the card ignores.

That is also the answer to "is addressing these safe" -- it was already being
done. The wipe's sweep zeroes 56/57/62/63 through the addressed path, and the
clone's conversion path writes 56/57 addressed and watches the UID follow,
which is how a gen1 card on the gen2 path is detected at all.

TWO FIGURES AT THE WRITE HELPER WIDEN HERE, both because the bench ran again
and not because either was wrong. The UID moving immediately on a write to 56
was two chips when the data-block writes were addressed and the SLIX-S run above
is the third. And the address enforcement was four of five cards, the blank
being TI Tag-it, which had never been sent a mis-addressed frame; sent one now,
with the OPTION flag set in both so the address was the only variable, it
answers silence to the wrong UID and 00 78 F0 to the right one. Five of five.

THE ONE THING THE SEQUENCE COSTS is a re-address in the middle. Block 56 carries
uid[7..4] and takes effect immediately -- no power-cycle. So by the time block
57 goes out the card has stopped answering to the address the frame before it
used. Without the re-address, 57 is sent to a card that is no longer there and
the run ends with half a UID written: neither the identity the card had nor the
one asked for. The unaddressed form had no such seam.

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
required: five cards take the write to block 56 with no unlock or commit in
front of it at all, and neither frame has ever been accepted by anything -- and
0x10 means "block not available" rather than "refused", which reads as those two
addresses not existing on this silicon. They stay regardless -- proxmark sends
them, and the cards that would need them are ones nobody here has -- but the
comment at their definition no longer claims a locked state before them.

BENCHED on all three gen1 chips. A Write UID to a target differing from each
card's own UID in BOTH HALVES, plain Success on each and the whole UID read back
-- not the half that a broken re-address leaves, which is why the target was
chosen that way: block 56 carries the printed tail and 57 the printed head, so a
run that wrote 56 and lost 57 would have left the card's own head behind. DSFID
survived per card, so nothing was written outside the UID registers.

The frame builder is the standard one now, because a gen1 backdoor write IS a
WRITE BLOCK: the address is what makes it magic, not the command. The response
is read for one thing, whether the card asked for the OPTION flag, and the
send's return stays discarded -- the 62/63 refusals are expected and the UID
moves regardless, so acting on them would abort a run that worked.

== THE WIPE HAZARD IS EVERY GEN1 CARD, NOT ONE SOMEONE ARMED ==

The notes and the screens scoped the wipe's identity risk to "a card left
ARMED by an earlier gen1 UID write", at six places, three of them user-facing.
There is nothing under that qualifier.

Five cards here take a write to block 56 with NO unlock or commit in front of it
at all, one of them a card this app had never written, and neither frame has
ever been accepted by anything. Those registers are simply writable. So the
condition is not the card's history but gen1 plus a claim high enough for the
sweep to reach 56/57, which is the reach rule and is already stated once in the
poller.

It matters because it UNDERSTATED the hazard, and in the direction that costs
a user a card: nobody can know whether a tag was written before they got it,
and nothing can detect it, so a warning aimed at that condition asks them to
worry only if they know something unknowable -- while every gen1 card measured
behaves the same way.

Nothing is claimed in the other direction. There may well be a lock state;
an acceptance of unlock or commit would have proved one, and a refusal proves
nothing, since a card locked or unlocked before it arrived is indistinguishable
from one that never was. The frames stay. What comes out is a precondition
asserted on the hazard. The two places that RECORD an observation keep what
they observed.

One more release-notes line goes with it, and it is older than this work:
"block contents are not compared" was true until the OPTION read-back, which
compares exactly that on a card whose acknowledgement it cannot collect. The
rule still holds and now carries its exception.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
