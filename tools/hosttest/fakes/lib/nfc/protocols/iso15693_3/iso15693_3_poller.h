// Host-side stand-in for the SDK's iso15693_3_poller.h, for tools/hosttest only.
// The implementations are the fake tag in fake_tag.c.
#pragma once

#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>
#include <toolbox/bit_buffer.h>

// Opaque to the poller; the fake ignores the pointer and answers from the one global fake tag.
typedef struct Iso15693_3Poller Iso15693_3Poller;

typedef enum {
    Iso15693_3PollerEventTypeError,
    Iso15693_3PollerEventTypeReady,
} Iso15693_3PollerEventType;

typedef struct {
    Iso15693_3PollerEventType type;
} Iso15693_3PollerEvent;

Iso15693_3Error iso15693_3_poller_inventory(Iso15693_3Poller* instance, uint8_t* uid);

Iso15693_3Error iso15693_3_poller_read_block(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint8_t block_number,
    uint8_t block_size);

Iso15693_3Error iso15693_3_poller_write_block(
    Iso15693_3Poller* instance,
    const uint8_t* data,
    uint8_t block_number,
    uint8_t block_size);

Iso15693_3Error
    iso15693_3_poller_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data);

// Raw frame send. The magic backdoor writes go out this way and the tag is not required to answer, so the
// poller ignores the result by design. Typed like the SDK's, so an argument-order slip fails to compile;
// the fake DECODES tx to decide whether a magic card accepts the UID.
Iso15693_3Error iso15693_3_poller_send_frame(
    Iso15693_3Poller* instance,
    const BitBuffer* tx,
    BitBuffer* rx,
    uint32_t fwt);
