NFC Magic ISO15693: the failure bitmap checks the index it is given

mark_failed and unmark_failed set and clear one bit of the 256-bit failure
bitmap, and took any index. Every caller passes one in range, but only because
each caller keeps it there; one past the end would write into the poller
struct beside the bitmap. Both now furi_check the index against
ISO15693_POLLER_MAX_BLOCKS, the bitmap's size, as the SDK's own block accessor
checks the index it is given -- so a slip at a call site is a crash at the
write rather than a poller corrupted somewhere else.

And the two calls to iso15693_3_get_block_data that passed a uint16_t block
number to its uint8_t parameter now cast it, so the narrowing is visible where
it happens, as the third call to it already was. Neither changes a value: one
loops over a source clamped to 256 blocks, the other over the four gen1
registers.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
