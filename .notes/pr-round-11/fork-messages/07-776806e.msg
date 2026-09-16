NFC Magic ISO15693: block 57 needs one more silent block than 56

Pre-existing, in the wiped == 0 branch of the wipe.

ISO15693_POLLER_WIPE_ABSENT_RUN is 8, so a card silent from block A is attempted
through A+7. That puts block 56 in reach from A >= 49 and block 57 only from
A >= 50. The comment treated the pair as atomic and justified it with "a write
that lands resets the run" -- which is exactly what cannot have happened inside
the branch it documents, since that branch is reached when no write landed at
all. At A == 49 the writes reach 56 and stop there.

"Only two things stop the sweep short of 56/57" also omitted the clock, which
stops it whenever the cut falls below 56. Named as a third case, with the
ISO15693_POLLER_PASS_MAX_MS backstop pointer.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
