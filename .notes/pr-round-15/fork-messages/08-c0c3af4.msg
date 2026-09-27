NFC Magic ISO15693: the gen2 frames cannot be addressed, and the 2.3 release notes

Two decisions, and they arrive together because the first rewrites one of the
notes: a release note that argued a case now states a measurement, and the
comment it rests on has to be in the same tree as it.

== THE GEN2 BACKDOOR CANNOT BE ADDRESSED ==

Every other frame set in this feature now carries the card's UID. This one does
not, and the reason given for that has been replaced.

It said 0xE0 is proprietary, so a conforming tag rejects it on the command and
there is no standard frame for a bystander to take. That is an argument about
CONFORMING tags. The population at risk is other magic cards, which parse
0xE0 09 exactly as the target does -- and the sequence programs the
configuration register as well as the UID, so a bystander gen2 card would come
away with a different block count, block size and IC reference on top of a
different identity.

MEASURED on all four gen2 cards here, and the answer is that it is not a choice.
Each takes the unaddressed form and refuses the addressed one -- with the
CORRECT UID, with and without the OPTION flag, and with the address bit set but
no UID in the frame. All four also accept an addressed ordinary WRITE BLOCK and
go silent on a one-byte-wrong address, so the frames are well formed and the
cards' addressing works. The backdoor is simply not reachable that way. There is
no addressed form of this command to send.

The frames therefore stay as they are, and the limit is stated rather than
argued away: another gen2 magic card in the field takes them and nothing here
can prevent it. Narrower than the gen1 hazard, because a tag that is not a gen2
magic card rejects the command outright. Not zero.

THE FLAGS BYTE IS A LITERAL rather than the shared helper, and the same run is
why. With OPTION set the card applies the write and answers nothing -- on all
four, including two that do not want the flag for ordinary writes. A run that
had turned the flag on for a card that does want it would lose the
acknowledgement here, so this sequence must not pick it up.

== THE 2.3 RELEASE NOTES ==

Every user-facing change in the round, written once, at the end.

WHAT IS NEW in the notes: every write that can carry the card's address now
does; a card that needs the OPTION flag gets it; a clone onto a gen1 card keeps
the card's identity whichever path it took; a clone says what it left behind; a
gen1 clone that lost nothing reports plain success; and the addressed-write
validation, seven cards over four identified chips.

WHAT IS CORRECTED: the clone bullet claimed a copy advertises the same chip
without qualification, which is the gen2 path only; the Partial claim for a gen1
clone is now conditional; the wipe hazard is no longer scoped to a card someone
previously armed, because five cards take a write to block 56 with no unlock or
commit in front of it at all; and the gen3 note says which half of its warning
is observed here.

WRITTEN AS ONE COMMIT DELIBERATELY. Release notes are a release artifact, not a
running log. Written per commit they are rewritten by later commits in the same
push -- three bullets for what is one fact to a user, then merged; a bullet
reworded twice as the bench widened. The reviewer reads the delta between
rounds, so text that appears and is rewritten inside a single push is pure cost
to him.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
