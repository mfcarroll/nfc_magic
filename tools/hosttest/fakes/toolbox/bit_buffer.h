// Host-side stand-in for the SDK's bit_buffer.h, for tools/hosttest only.
// The sweep sends no frames; these exist so the backdoor-UID paths in the same translation unit link.
#pragma once

#include <furi.h>

typedef struct BitBuffer BitBuffer;

BitBuffer* bit_buffer_alloc(size_t capacity);
void bit_buffer_free(BitBuffer* buf);
void bit_buffer_reset(BitBuffer* buf);
void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte);
