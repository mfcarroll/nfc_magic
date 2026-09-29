NFC Magic ISO15693: gen2 frames cannot be addressed, and the 2.3 release notes

Two decisions, and they arrive together because the first rewrites one of the
notes: a release note that argued a case now states a measurement, and the
comment it rests on has to be in the same tree as it.

== THE GEN2 BACKDOOR CANNOT BE ADDRESSED ==

Every other write in this feature now carries the card's UID. This one does not,
and the reason is now a measurement rather than an argument.

The argument was that 0xE0 is proprietary, so a conforming tag rejects it on the
command and there is no standard frame for a bystander to take. That covers
CONFORMING tags. The population at risk is other magic cards, which parse 0xE0
09 exactly as the target does -- and the sequence programs the configuration
register as well as the UID, so a bystander gen2 card would come away with a
different block count, block size and IC reference on top of a different
identity.

MEASURED on all four gen2 cards here, and the answer is that it is not a choice.
Each takes the unaddressed form and refuses the addressed one -- with the
CORRECT UID, with and without the OPTION flag, and with the address bit set but
no UID in the frame. All four also accept an addressed ordinary WRITE BLOCK and
go silent on a one-byte-wrong address, so the frames are well formed and the
cards' addressing works. The backdoor is simply not reachable that way. There is
no addressed form of this command to send.

The frames therefore stay as they are, and the limit is stated rather than
argued away: another gen2 magic card in the field takes them and nothing here
can prevent it. Narrower than an unaddressed standard write, which any writable
tag in the field takes. Not zero.

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
gen1 clone whose file stops below block 56 is not Partial on that account; and
the addressed-write validation, seven cards over four identified chips.

WHAT IS CORRECTED: the clone bullet claimed a copy advertises the same chip
without qualification, which is the gen2 path only; the wipe bullet named 62/63
the unlock and commit registers, and calls the four the UID / backdoor
registers, since what 62/63 do is inferred; the Partial claim for a gen1 clone
is now conditional; "block contents are not compared" now carries its exception,
the OPTION read-back, which compares exactly that on a card whose
acknowledgement it cannot collect; the wipe hazard is no longer scoped to a card
someone previously armed, since five cards took a write to block 56 with nothing
in front of it and a card's history cannot be known; the gen3 note said nothing
had been observed, when one tested tag has a writable UID register at 0x10; it
now says the bricking was not tried in testing, and so does the code comment
behind the wipe confirmation's gen3 line; "the one result that proves the card
is magic" drops "the one", since a verified Write UID proves it too; the
over-capacity note says the file is larger than the card, since on gen1 the card
goes on reporting its own count, and "written and acknowledged" gains the card
that acknowledges none and is read back instead; the wipe's reach loses its
pointer into the source, which a user cannot follow; the progress line names
"Wiping" for a wipe; and the bullet saying every write reaches every tag in the
field becomes the gen2 limit plus its own bullet for keeping one tag in the
field, since every other write is now addressed and what remains is the reads
and the inventories.

ONLY THIS COMMIT TOUCHES THE RELEASE NOTES. Each note describes the end state,
so it belongs to the last commit that changes the behaviour it describes, and no
earlier commit here carries one.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
