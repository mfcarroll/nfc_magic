NFC Magic ISO15693: one clamp for block size, across all three callers

The clone clamped iso15693_3_get_block_size against ISO15693_MAX_BLOCK_SIZE, the
wipe clamped the same quantity against sizeof(zeros), and the gen2 CFG derivation
spelled it a third time. The first two agreed only because that array is declared
from the same macro -- a coincidence the code did not state.

One helper, against the MACRO, and it says why the three are not one case:

  - the CLONE's value comes out of a loaded .nfc. The SDK's loader gates
    allocation on block_size > 0 but loads either way, so 0..255 arrives and this
    is the only bound before a 32-byte stack buffer.
  - the gen2 CFG derivation reads the same field off the same source, and is the
    one that matters most: those bytes are programmed into the card and outlive
    the run.
  - the WIPE's comes off the wire, where Get System Info carries (size - 1) in a
    five-bit field, so it is at most 32 on arrival and cannot fire.

The one that looks redundant is not the one that is, which is why they share this
rather than carrying three notes. The clone site keeps only what is its own: that
this is the SOURCE's geometry, which a gen1 target does not adopt.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
