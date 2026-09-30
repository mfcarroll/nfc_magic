NFC Magic ISO15693: comments from earlier rounds that said something false

A read of everything this PR ships, once, against the code and the measurement
records. Round 15's own lines were corrected in the commits that wrote them;
these were in the tree before it, so they are corrected here. Nothing here
changes behaviour except one string.

WHAT THE CODE DOES, where a comment said otherwise:

- the gen1 clone contract said such a clone is never a clean Success. A source
  with no data at 56/57/62/63 loses nothing there, whether it ends before block
  56 or reaches them with nothing stored, and can end in Success, as the field
  docs and the release notes already say. Its emit list gains Success, and the
  Partial event scopes "fell back to gen1" to a source holding data at those
  blocks.
- the Fail event's list of causes missed one: a clone whose UID took but whose
  every data block was refused, which reports Fail too.
- the partial-details bound said a wipe's total excludes the backdoor blocks.
  Only a gen1 clone's does; a wipe's stops at the highest block proven present.
- a worked example in the partial-details scene -- a card claiming 200 blocks
  and holding 10, stopped by the time limit -- counted 170 blocks attempted and
  filed it as a cut above the claim. The header takes the same card to a cut at
  block 150: below the claim, with 140 blocks attempted that answered nothing.
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
  showed: the four-run probe was one gen2 card, a plain NXP ICODE tag was sent
  writes past its end and its last block was unchanged, full read sweeps of two
  more cards stop at their top, and two tested tags mirror their cells and serve
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
