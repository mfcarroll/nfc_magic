// Host-side stand-in for the SDK's bit_buffer.h, for tools/hosttest only.
//
// These store real bytes rather than discarding them, because the fake tag DECODES the magic backdoor
// frames the poller builds -- that is what lets a fake gen2-magic card actually change its UID and a
// non-magic one refuse to, which is the whole subject of the write state machine.
#pragma once

#include <furi.h>

#define FAKE_FRAME_CAP (64U)

typedef struct {
    uint8_t data[FAKE_FRAME_CAP];
    size_t size;
} BitBuffer;

BitBuffer* bit_buffer_alloc(size_t capacity);
void bit_buffer_free(BitBuffer* buf);
void bit_buffer_reset(BitBuffer* buf);
void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte);
void bit_buffer_append_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes);
size_t bit_buffer_get_size_bytes(const BitBuffer* buf);
uint8_t bit_buffer_get_byte(const BitBuffer* buf, size_t index);
