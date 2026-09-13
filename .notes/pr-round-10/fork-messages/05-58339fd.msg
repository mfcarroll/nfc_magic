NFC Magic ISO15693: three header contracts that the scenes state more carefully than the header

In each case the correct version was already in the tree, in the file that consumes
the contract.

Partial's contract said a run cut short by the wall-clock bound is Partial,
unqualified. A wipe cut short having cleared nothing reports Fail: the wiped == 0
short-circuit runs before success_or_partial sees pass_truncated. That is the card
the bound was written for -- one that refuses every write and answers every read, so
it accumulates no absences, walks to the ceiling and is cut with nothing wiped. The
write-fail scene already says so.

capacity_confirmed's doc gave only "scattered or anomalous" as the false case and
never mentioned the cut, though !pass_truncated is a conjunct of
failures_are_top_tail and the .c spends twenty lines on what that costs. A scene
author reads the field doc, not the predicate.

blocks_done "converges on blocks_total for a clone" is deliberately false for a cut
clone: the .c argues that the last frame must not jump to 100%, because that would
show a completed write that was cut.

Comment-only.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
