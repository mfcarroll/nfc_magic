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
writes the target UID back through the gen1 sequence. The switch is made as soon
as the UID moves, before the write's own result is read -- the move is what
shows the write landed, acknowledged or not. The end state is the one an
ordinary gen1 clone produces -- a shape already tested and already reported
correctly -- rather than a new one.

THAT WRITE IS NEITHER A BLOCK WRITTEN NOR A FAILURE. It went to a register, not
to memory. Counted as a success it would stop a card smaller than the file being
reported too small, since a register answers whatever the card's size. Counted
as a failure -- its acknowledgement lost, and its retries sent to the address
the card has just left -- it would make every later block look like memory
written above a failure. Nor is it one of the blocks the progress figure counts:
the conversion takes all four out of the total, so the write comes off the count
as well, or a pass the clock cut above them would read one past its own total. A
failure the pass recorded at one of the four before the UID moved is taken back
from the count it went into: a register answers no read, so an empty one was
counted as past the card's capacity, not as a failure. What that failure implied
goes with it: a block written above it no longer counts as memory written above
a failure, which alone would deny a real capacity edge further up.

AND THE UID IS READ BACK BEFORE ANYTHING IS REPORTED. Whether the run noticed
the move rests on an inventory that can miss, or be answered by a second tag in
the field, and the repair's frames each go out once, so one lost frame leaves
the card answering to the file's bytes after all. So a gen2-path clone that sent
a frame to 56/57 ends in a verify state of its own, behind a field power-cycle
like the other UID verifies. The target reports the run as it stands, any other
UID is printed as unexpected, and no answer is a lost card, whose Details says
the UID was not re-checked, as a wipe's does. So does a card lost mid-pass once
the pass has sent 56/57 a frame: the re-read was due there too, and the pass
learns the card has gone only at its end. On a gen2 card those blocks are
memory, and the check costs a power-cycle and the activation after it. The
popup's last progress frame comes after the check, so it does not read 100%
while the card is still being asked something.

NOT BY SKIPPING those blocks whenever the UID already matches, which is the
shorter fix and is wrong: on a real gen2 card re-cloned from a DIFFERENT image
that happens to share its UID, 56/57/62/63 are ordinary memory. The skip deducts
them from the total, and the run reports a clean "Cloned 60/60" over four blocks
of the file it chose not to write -- trading a silent data loss for a rare
identity one.

NOTHING ON SCREEN SAYS ANY OF THAT, and that is deliberate. What the user needs
is what the CARD is, and the run reports exactly as a gen1 clone of the same
file would. A note about the identity being disturbed and put back would be the
app narrating its own process.

THE CONSENT CALL, named because it is one. The repair sends the whole gen1
sequence, unlock and commit included, and the gen1 opt-in screen is not shown on
this path. That screen exists to warn that gen1 writes destroy four blocks of
user data on a tag that is NOT gen1; this card has just proved that it is, by
moving its UID to the value our write implied, so 56/57 are registers -- and on
every gen1 chip measured 62/63 are too -- rather than anyone's data. Leaving the
card answering to bytes lifted out of the file is the worse outcome by a
distance.

It rests on the re-address the data-block addressing introduced, which accepts
only the UID a write implies; this adds the flag that records the move.

COMMENTS THIS WOULD LEAVE FALSE change with it. The branch's own note took a
card already wearing the target UID to be the one the source was read from; it
now names the two that reach it. The COUNT_OF note counted three loops over the
backdoor list, and this adds a fourth. The note on that list counted the places
that ask it, and this adds one, so it names them without a count. The
mark/unmark note called every clear the else-arm of a two-way decision, and the
repair's clear is not. The note on the reporting state counted the fields
get_result leaves out, and this adds one, so it says so where it is declared.
The write-fail scene said a clone has no identity check to skip, and it has one
now. The unexpected-UID notes -- at the result field, at the Fail event and at
the reason -- named the gen2 backdoor as the only route there, and they name the
re-read too. And the write-state switch's note named the worst of four states to
land in by accident; there are five.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
