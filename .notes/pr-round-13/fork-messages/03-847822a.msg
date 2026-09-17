NFC Magic ISO15693: one clamp for block size, and only one caller can trip it

The clone clamped iso15693_3_get_block_size against ISO15693_MAX_BLOCK_SIZE; the
wipe clamped the same quantity against sizeof(zeros). They agreed only because
that array is declared from the same macro -- a coincidence the code did not
state, and one nothing obliges the next buffer to preserve.

iso15693_poller_clamp_block_size is both, against the MACRO. What the note is
careful about is that the two callers are opposites rather than one case:

  - the CLONE's value comes out of a loaded .nfc. The SDK's loader checks it is
    non-zero and nothing else, so 1..255 arrives and this is the only bound
    between a hand-edited file and a 32-byte stack buffer.
  - the WIPE's comes off the wire, where Get System Info carries (size - 1) in a
    five-bit field, so it is 1..32 on arrival and the clamp cannot fire.

Saying "neither controls its input" would flatten that into an invitation to
delete one, and the one that looks redundant is not the one that is. Both clamp
regardless, so the sites cannot drift.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
