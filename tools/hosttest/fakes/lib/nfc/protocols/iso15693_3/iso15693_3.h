// Host-side stand-in for the SDK's iso15693_3.h, for tools/hosttest only.
//
// The layout does NOT have to match the SDK's, because the accessors below are fakes too -- it only
// has to be self-consistent. What DOES have to match is the semantics the poller depends on; those are
// noted where they matter.
#pragma once

#include <furi.h>

#define ISO15693_3_UID_SIZE (8U)

// 256 block numbers (the ISO15693 block-number space) at up to 32 bytes each.
#define FAKE_MAX_BLOCKS     (256U)
#define FAKE_MAX_BLOCK_SIZE (32U)

// Mirrors lib/nfc/protocols/iso15693_3/iso15693_3.h member-for-member and in order, so the values match
// the SDK's as well as the names. The callers only ever compare against None, but the poller's own
// response parse PRODUCES these, so here the values have to be right rather than merely distinct.
//
// Iso15693_3ErrorInternal is what a refused write actually produces: BLOCK_UNAVAILABLE / BLOCK_LOCKED /
// BLOCK_ALREADY_LOCKED / BLOCK_WRITE / BLOCK_LOCK all map to it, in iso15693_3_error_response_parse
// (iso15693_3_i.c:42-48) and in the app's iso15693_poller_parse_write_response alike. It is a
// well-formed error RESPONSE, not silence.
typedef enum {
    Iso15693_3ErrorNone,
    Iso15693_3ErrorNotPresent,
    Iso15693_3ErrorBufferEmpty,
    Iso15693_3ErrorBufferOverflow,
    Iso15693_3ErrorFieldOff,
    Iso15693_3ErrorWrongCrc,
    Iso15693_3ErrorTimeout,
    Iso15693_3ErrorFormat,
    Iso15693_3ErrorIgnore,
    Iso15693_3ErrorNotSupported,
    Iso15693_3ErrorUidMismatch,
    Iso15693_3ErrorFullyHandled,
    Iso15693_3ErrorUnexpectedResponse,
    Iso15693_3ErrorInternal,
    Iso15693_3ErrorCustom,
    Iso15693_3ErrorUnknown,
} Iso15693_3Error;

// Copied value-for-value from the SDK's public iso15693_3.h, and identical there across Momentum,
// Unleashed, RogueMaster, Xero and official. The poller builds its own addressed WRITE BLOCK frame out
// of these and parses the response with them, so a drift here would be a test passing against a frame
// no card would answer.
#define ISO15693_3_REQ_FLAG_SUBCARRIER_1 (0U << 0)
#define ISO15693_3_REQ_FLAG_DATA_RATE_HI (1U << 1)
#define ISO15693_3_REQ_FLAG_T4_ADDRESSED (1U << 5)

#define ISO15693_3_RESP_FLAG_NONE  (0U)
#define ISO15693_3_RESP_FLAG_ERROR (1U << 0)

#define ISO15693_3_RESP_ERROR_NOT_SUPPORTED        (0x01U)
#define ISO15693_3_RESP_ERROR_FORMAT               (0x02U)
#define ISO15693_3_RESP_ERROR_OPTION               (0x03U)
#define ISO15693_3_RESP_ERROR_UNKNOWN              (0x0FU)
#define ISO15693_3_RESP_ERROR_BLOCK_UNAVAILABLE    (0x10U)
#define ISO15693_3_RESP_ERROR_BLOCK_ALREADY_LOCKED (0x11U)
#define ISO15693_3_RESP_ERROR_BLOCK_LOCKED         (0x12U)
#define ISO15693_3_RESP_ERROR_BLOCK_WRITE          (0x13U)
#define ISO15693_3_RESP_ERROR_BLOCK_LOCK           (0x14U)
#define ISO15693_3_RESP_ERROR_CUSTOM_START         (0xA0U)
#define ISO15693_3_RESP_ERROR_CUSTOM_END           (0xDFU)

#define ISO15693_3_CMD_WRITE_BLOCK (0x21U)

#define ISO15693_3_SYSINFO_FLAG_DSFID  (1U << 0)
#define ISO15693_3_SYSINFO_FLAG_AFI    (1U << 1)
#define ISO15693_3_SYSINFO_FLAG_MEMORY (1U << 2)
#define ISO15693_3_SYSINFO_FLAG_IC_REF (1U << 3)

typedef struct {
    uint8_t flags;
    uint8_t dsfid;
    uint8_t afi;
    uint8_t ic_ref;
    uint16_t block_count;
    uint8_t block_size;
} Iso15693_3SystemInfo;

typedef struct {
    uint8_t uid[ISO15693_3_UID_SIZE];
    Iso15693_3SystemInfo system_info;
    uint8_t block_data[FAKE_MAX_BLOCKS * FAKE_MAX_BLOCK_SIZE];
} Iso15693_3Data;

Iso15693_3Data* iso15693_3_alloc(void);
void iso15693_3_free(Iso15693_3Data* data);
void iso15693_3_copy(Iso15693_3Data* dest, const Iso15693_3Data* src);
void iso15693_3_reset(Iso15693_3Data* data);

uint16_t iso15693_3_get_block_count(const Iso15693_3Data* data);
uint8_t iso15693_3_get_block_size(const Iso15693_3Data* data);
// The SDK's version furi_check()s the index; iso15693_poller_block_held_data's range check is
// documented as relying on that, so the fake must abort the same way rather than return NULL.
const uint8_t* iso15693_3_get_block_data(const Iso15693_3Data* data, uint8_t block);
