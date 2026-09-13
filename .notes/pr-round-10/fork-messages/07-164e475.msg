NFC Magic ISO15693: a wipe that loses the card never said the identity check had not run

A wipe that loses the card reports CardLost and returns before
Iso15693WriteStateVerifyWipe is entered, so uid_verified stays false and nothing ran
to notice. has_details sent CardLost to its default arm, so there was no Details
button, and "UID not re-checked" -- the only sentence in the app that says the check
did not reach an answer -- is reachable only through Details. The user was told the
card had been removed, and nothing else.

The screen it matters on is an armed gen1 card large enough for the sweep to reach
56/57, where those two blocks ARE the UID: the card can be gone and its identity
with it. The scene called the case moot because the card had left; the reason it is
not moot was ten lines below, in the same comment block.

Three parts, because the one-line fix would have shipped a misleading screen:

  has_details gains CardLost, gated on wipe mode AND !uid_verified. A clone has no
  identity check to skip, and a wipe that lost the card after the check answered has
  nothing left to report.

  The note's wording is chosen by route. "did not answer after the field reset"
  names a reset that never happened here; a card-lost wipe gets "stopped answering
  before the identity check could finish". VerifyWipe does not downgrade to CardLost
  on silence -- it logs and falls through -- so the original wording still belongs
  to the success and partial reasons.

  The block list is suppressed. The sweep's own card-lost check says a lifted card
  "can surface as a pile of blocks that wouldn't clear", and those counters are the
  caller's to discard on that exit. list_upto = 0 does it and takes has_block_list
  with it, so the title falls to "Wipe notes" and the UID note is the whole screen.

CardLost is now in both is_retryable and has_details, which is the pairing that
produced the round-5 "a control labelled Exit opened the Details scroll view" bug.
Re-read against the rule at the button block: on_enter and on_event both test
has_details first, in the same order.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
