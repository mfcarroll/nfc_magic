NFC Magic ISO15693: the sweep's reach is a closed form, stated once

THREE gates must all pass before the wipe's loop can break: the absent run has to
reach ISO15693_POLLER_WIPE_ABSENT_RUN, the advertised range has to have been
attempted, and the run has to SURVIVE the re-probe. Below the claim the trip
falls through to continue -- the advertised-count floor doing its job -- so a run
of eight alone does not end it.

With A the first block of the FINAL unbroken run of silence, and claim the
advertised count:

    L = min(max(A + 7, claim - 1), ISO15693_POLLER_WIPE_MAX_BLOCKS - 1)
    56 reached  <=>  A >= 49  OR  claim >= 57
    57 reached  <=>  A >= 50  OR  claim >= 58

The third gate is why A is the run the sweep ENDS on rather than the first
silence anywhere: wipe_note_present zeroes absent_run from three sites -- write
success, read success and the re-probe -- and only the first increments wiped, so
on the branch where nothing was cleared a run can still have been reset. A card reading 0-4, silent at 5, reading 6-59 and silent
from 60 reaches 56 and 57; taking A as the first silence would say it does not.

Stating it once here, with the release notes carrying only the qualitative half
-- a card that keeps answering reads walks past 56/57 whatever it claims -- is
what stops this being paraphrased differently in each place that mentions it.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
