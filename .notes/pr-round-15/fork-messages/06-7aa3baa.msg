NFC Magic ISO15693: a clone that lands in a gen1 card's UID repairs it

The gen2 verify proves the card's UID MATCHES the target, not that the card is
magic, which is what the poller's comment says at that branch. A card already
wearing the target UID satisfies it without anything having happened -- and
re-cloning the same file is exactly how a card comes to wear it.

MEASURED. A 64-block source cloned twice onto a gen1 SLIX. The first run took
the gen1 path and was correct. The second found the card already wearing the
source's UID, so the gen2 verify passed, the data pass ran with the backdoor
blocks IN scope, and the file's blocks 56 and 57 landed in the UID registers.
The card was left answering to 39 A5 39 5A 38 A5 38 5A -- the file's own bytes
through the UID mapping -- under a screen reading "Cloned 30/64, Not written:
34" and nothing else.

WHAT CONVERTS IT. A write to 56/57 that moves the UID to precisely the value
that write implies can only mean those addresses are registers, so the card is
gen1 whatever path the run took. The write that does the damage is the same
event that identifies the card, so the run switches: it stops feeding the file
into the registers, keeps going with everything above them, and afterwards
writes the target UID back through the gen1 sequence. The end state is the one
an ordinary gen1 clone produces -- a shape already tested and already reported
correctly -- rather than a new one.

NOT BY SKIPPING those blocks whenever the UID already matches, which is the
shorter fix and is wrong: on a real gen2 card re-cloned from a DIFFERENT image
that happens to share its UID, 56/57/62/63 are ordinary memory. The skip deducts
them from the total, and the run reports a clean "Cloned 60/60" over four blocks
of the file it chose not to write -- trading a silent data loss for a rare
identity one.

NOTHING ON SCREEN SAYS ANY OF THAT, and that is deliberate. What the user needs
is what the CARD is -- gen1, with the file's data absent from those four
addresses -- and the existing gen1 caveat says exactly that. A note about the
identity being disturbed and put back would be the app narrating its own
process.

THE CONSENT CALL, named because it is one. The repair sends the whole gen1
sequence, unlock and commit included, and the gen1 opt-in screen is not shown
on this path. That screen exists to warn that gen1 writes destroy four blocks
of user data on a tag that is NOT gen1; this card has just proved that it is,
by moving its UID to the value our write implied, so those addresses are
registers rather than anyone's data. Leaving the card answering to bytes
lifted out of the file is the worse outcome by a distance.

THE RE-ADDRESS CHECKS ITS ANSWER NOW. It took whatever the inventory returned,
and that inventory is the SDK's 1-slot unaddressed one, so a second tag in the
field can answer it (#251), and re-addressing to a stranger points every later
frame at the wrong card. Because we know what we wrote -- 56 carries uid[7..4],
57 carries uid[3..0] -- there are exactly two honest replies: unchanged, or what
the write implies. A third is refused and the address held. Not airtight, and
unfixably so: a bystander holding the predicted UID would pass, because magic
cards make UIDs non-unique and a 1-slot inventory cannot tell two cards apart.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
