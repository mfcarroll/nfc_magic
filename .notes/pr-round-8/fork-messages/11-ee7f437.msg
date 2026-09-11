NFC Magic ISO15693: the poller owns the wiped == 0 argument; the scene keeps its own half

The details scene carried a copy of the poller's reasoning about the wiped == 0
short-circuit -- same clauses, same order: "the one inference this file declines
to draw anywhere else", "three WRITE BLOCKs each at 56 and 57 before giving up",
"a tag can apply a write without answering", "reports Wipe failed, never runs the
check, and never says the check did not run". 9520e177, one commit earlier in the
round-7 push, removed three duplications of exactly this shape.

The poller keeps it, because the short-circuit is the poller's control flow.

The copy also broke on the move, which is what duplication does. "The one
inference THIS FILE declines to draw anywhere else" was written about
iso15693_poller.c; in a scene that renders text and draws no inference about
writes it points at nothing. And the dispositions had drifted -- "left as it is"
in the poller against "Filed rather than fixed" here, one decision in two
wordings. Deleting the copy settles both.

What stays is what this file is the authority on: the six wipe reason codes and
which of them reach a Details route, and that on an armed gen1 card the UID
registers are overwritten long before any plausible cut, so "Wipe stopped" can
offer Retry without saying the identity check never reached an answer. That is
about what a button promises, not about the poller's control flow.

Deletion only, net -4 comment lines, no code.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
