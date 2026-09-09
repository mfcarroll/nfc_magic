NFC Magic ISO15693: the wiped==0 path does not prove the UID is safe

The last comment in the file that claims more than the code does, and the one
mishamyte had already disproved in Round 6 without asking for a code change.

"Skip the power-cycle, since no write landed that could have moved the UID" is
the one inference this file declines to draw anywhere else. The sweep sent three
WRITE BLOCKs each at 56 and 57 before giving up, and write_identity's own comment
says a tag can apply a write without answering. So on an ARMED gen1 card this
path can move the UID, report "Wipe failed", never run the check, and never say
the check did not run.

You said the wiped == 0 short-circuit is pre-existing and not this PR's to change,
so it is left exactly as it is. What changes is that the comment now says what
skipping the check costs instead of asserting it costs nothing -- and names this
as the one path where the mitigation #255 describes does not run at all.

Comment +5, code +0, which is the right direction for a correction even in a
delta about cutting: Round 6 established that fixing a false claim costs lines.
106 host tests before and after.

