NFC Magic ISO15693: the sweep's reach is a closed form, stated once

TWO gates must pass before the wipe's loop can break: the absent run has to
reach ISO15693_POLLER_WIPE_ABSENT_RUN, and the advertised range has to have been
attempted. Below the claim the trip falls through to continue -- the
advertised-count floor doing its job -- so eight absences alone do not end it.
With A the first block that answers nothing and claim the advertised count:

    L = max(A + 7, claim - 1)
    56 reached  <=>  A >= 49  OR  claim >= 57

The comment modelled the first term only, so it said a card answering no read at
all but advertising 57 or more cannot reach 56/57. It does, three WRITE BLOCKs
go out at each, and that is the wrong direction on the one path where the UID
check never runs.

Corrected in the same passage: the run resets on any block that ANSWERS, not
only on a write that lands. wipe_note_present is called from the read-success
path and does not increment wiped, so a run can have been reset even on the
branch where nothing was cleared.

The release notes stop carrying the rule and keep only the qualitative half --
a card that keeps answering reads walks past 56/57 whatever it claims -- which
is the half that cannot be wrong. Stating it exactly in one place and
qualitatively in the other is what ends four rounds of paraphrase.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
