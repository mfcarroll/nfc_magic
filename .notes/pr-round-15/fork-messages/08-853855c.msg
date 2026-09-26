NFC Magic ISO15693: the 2.3 release notes for this round

Every user-facing change in the round, written once, at the end.

WHAT IS NEW in the notes: every write now names the card it is for; a card
that needs the OPTION flag gets it; a clone onto a gen1 card keeps the
card's identity whichever path it took; a clone says what it left behind;
a gen1 clone that lost nothing reports plain success; and the
addressed-write validation.

WHAT IS CORRECTED: the clone bullet claimed a copy advertises the same
chip without qualification, which is the gen2 path only; the Partial
claim for a gen1 clone is now conditional; the wipe hazard is no longer
scoped to a card someone previously armed, because five cards take a
write to block 56 with no unlock or commit in front of it at all; and
the gen3 note says which half of its warning is observed here.

WRITTEN AS ONE COMMIT DELIBERATELY. Release notes are a release artifact,
not a running log. Written per commit they are rewritten by later commits
in the same push -- three bullets for what is one fact to a user, then
merged; a bullet reworded twice as the bench widened. The reviewer reads
the delta between rounds, so text that appears and is rewritten inside a
single push is pure cost to him.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>

