NFC Magic ISO15693: the poller owns the wiped == 0 argument; the scene keeps its own half

The details scene carried a copy of the poller's reasoning about the wiped == 0
short-circuit -- same clauses, same order: "the one inference this file declines
to draw anywhere else", "three WRITE BLOCKs each at 56 and 57 before giving up",
"a tag can apply a write without answering", "reports Wipe failed, never runs the
check, and never says the check did not run". You point out that 9520e177, one
commit earlier in that push, removed three duplications of exactly this
shape.

You are right about which side should keep it: the short-circuit is the poller's
control flow, so the poller owns the argument for why it is a hazard rather than
a safe inference.

The copy also broke on the move, which is the thing duplication does. "The one
inference THIS FILE declines to draw anywhere else" was written about
iso15693_poller.c; in a scene that renders text and draws no inference about
writes it points at nothing. And the dispositions had already drifted apart --
"left as it is" in the poller against "Filed rather than fixed" here, one
decision in two wordings. Deleting the copy settles both.

What stays is what this file is actually the authority on: the six wipe reason
codes and which of them reach a Details route, and the consequence for the screen
-- on an armed gen1 card the UID registers are overwritten long before any
plausible cut, so "Wipe stopped" can offer Retry without saying the identity check
never reached an answer. That last one is about what a button promises, which is
this scene's business and not the poller's.

Deletion only, net -4 comment lines, no code.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
