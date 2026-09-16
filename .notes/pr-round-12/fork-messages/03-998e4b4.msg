NFC Magic ISO15693: one clamp for block size, against the macro rather than a buffer

The clone clamped iso15693_3_get_block_size against ISO15693_MAX_BLOCK_SIZE;
the wipe clamped the same quantity against sizeof(zeros). They agreed, but only
because that array is declared from the same macro -- a coincidence the code did
not state, and one nothing obliges the next buffer to preserve.

iso15693_poller_clamp_block_size is both, against the macro, and says why the
input needs clamping at all: neither caller controls it. The clone's comes out
of a loaded .nfc, hand-editable and unbounded by anything this app decides; the
wipe's comes off the wire in the card's own GET SYSTEM INFO. Both then feed a
fixed stack buffer.

The clone site keeps the one note the helper cannot carry -- that this is the
SOURCE's geometry, which a gen1 target does not adopt, so a mismatch there makes
every empty failure read as absent.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
