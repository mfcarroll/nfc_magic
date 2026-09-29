NFC Magic ISO15693: comments from earlier rounds that said something false

A read of everything this PR ships, once, against the code and the measurement
records. Round 15's own lines were corrected in the commits that wrote them;
these were in the tree before it, so they are corrected here. Nothing here
changes behaviour except one string.

WHAT THE CODE DOES, where a comment said otherwise:

- the gen1 clone contract said such a clone is never a clean Success. A source
  that ends before block 56 has nothing at 56/57/62/63 to lose, and gets one, as
  the field docs and the release notes already say. Its emit list gains Success,
  and the Partial event scopes "fell back to gen1" to a source that reaches
  those blocks.
- the Fail event's causes leave out the clone whose UID took and whose every
  data block was refused.
- the partial-details bound said a wipe's total excludes the backdoor blocks.
  Only a gen1 clone's does; a wipe's stops at the highest block proven present.
- the cut-index example put the header's own card on the wrong side of the
  claim, with 170 blocks where the header has 140.
- the reach rule said TWO gates and listed three.
- a pointer sent the reader to the header for a worst case stated beside
  ISO15693_POLLER_PASS_MAX_MS.
- the write scene's Fail comment named two of the seven routes.
- a cut wipe that cleared nothing is a Fail, not Partial.

MEASUREMENTS STATED AS LAWS:

- "a block that fails every write is past capacity", at the retry count and in
  the data pass -- which then read-probes such a block precisely because it may
  not be. The read decides. "Writes succeed well past it" gets its sample, the
  gen2 cards.
- "past capacity a read fails" is scoped where it is stated, with what each card
  showed: the four-run probe was one gen2 card, a plain NXP ICODE tag took
  writes past its end with its last block unchanged, full read sweeps of two
  more cards stop at their top, and one tested tag mirrors its cells and serves
  a read at every address. The data pass and the success rule point there, and
  the note about where the probe lives goes, since nothing names it now.
- "a ceiling, not a cost" was false for the two cards the next constants exist
  for, and 40-70ms is an estimate wherever it appears.
- a Retry after a cut clears only a transient.

OVERCLAIMS:

- "the one result that proves the card is magic", at the reason and in the
  poller: a verified Write UID proves it too.
- the over-capacity comments said the card advertises more blocks than it holds,
  which is gen2 only -- on gen1 the card reports its own count. What every path
  shares is the file being larger than the card.
- a card left "permanently" advertising one block: the next configuration write
  fixes it.

AND ONE USER-FACING STRING. The gen1 opt-in said gen1 "writes the UID to blocks
56/57/62/63". The UID goes to 56/57; the string now says gen1 writes those four
to set the UID, and the comments beside it call them the four gen1 registers.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
