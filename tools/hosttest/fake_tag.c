#include "fake_tag.h"

#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <toolbox/bit_buffer.h>
#include <stdarg.h>

uint32_t fake_tick = 0;
FakeTag fake_tag;
Iso15693_3Data fake_activation_cache;

#define FAKE_MARKER (0xA5U)
#define FAKE_LOG_CAP (16384U)

static char fake_log_buf[FAKE_LOG_CAP];
static size_t fake_log_len;

void fake_log(char level, const char* fmt, ...) {
    if(fake_log_len + 2 >= FAKE_LOG_CAP) return;
    int n = snprintf(fake_log_buf + fake_log_len, FAKE_LOG_CAP - fake_log_len, "[%c] ", level);
    if(n > 0) fake_log_len += (size_t)n;

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(fake_log_buf + fake_log_len, FAKE_LOG_CAP - fake_log_len, fmt, args);
    va_end(args);
    if(n > 0) fake_log_len += (size_t)n;

    if(fake_log_len + 1 < FAKE_LOG_CAP) fake_log_buf[fake_log_len++] = '\n';
    fake_log_buf[fake_log_len] = '\0';
}

const char* fake_log_text(void) {
    return fake_log_buf;
}

void fake_log_clear(void) {
    fake_log_len = 0;
    fake_log_buf[0] = '\0';
}

void fake_tag_reset(void) {
    memset(&fake_tag, 0, sizeof(fake_tag));
    memset(&fake_activation_cache, 0, sizeof(fake_activation_cache));
    fake_tag.tick_cost_per_op = 1;
    fake_tick = 0;
    fake_log_clear();
}

void fake_tag_init(uint16_t advertised, uint16_t physical, uint8_t block_size) {
    fake_tag_reset();
    fake_tag.advertised = advertised;
    fake_tag.block_size = block_size;
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; i++) {
        fake_tag.uid[i] = (uint8_t)(0xE0 + i);
    }
    for(uint16_t b = 0; b < FAKE_MAX_BLOCKS; b++) {
        fake_tag.kind[b] = (b < physical) ? FakeBlockWritable : FakeBlockAbsent;
    }
    if(physical > 0) fake_tag_fill(0, (uint16_t)(physical - 1), FAKE_MARKER);
    fake_tag_cache_from_activation();
}

void fake_tag_set_range(uint16_t first, uint16_t last, FakeBlockKind kind) {
    for(uint16_t b = first; b <= last && b < FAKE_MAX_BLOCKS; b++) {
        fake_tag.kind[b] = kind;
    }
}

void fake_tag_fill(uint16_t first, uint16_t last, uint8_t byte) {
    for(uint16_t b = first; b <= last && b < FAKE_MAX_BLOCKS; b++) {
        memset(fake_tag.content[b], byte, fake_tag.block_size);
    }
}

// Does this block answer at all right now? Absent blocks answer nothing; a lifted card answers nothing
// anywhere.
static bool fake_block_answers(uint16_t block) {
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) return false;
    if(block >= FAKE_MAX_BLOCKS) return false;
    return fake_tag.kind[block] != FakeBlockAbsent;
}

static void fake_charge_op(void) {
    fake_tag.ops++;
    fake_tick += fake_tag.tick_cost_per_op;
}

void fake_tag_cache_from_activation(void) {
    memset(&fake_activation_cache, 0, sizeof(fake_activation_cache));
    memcpy(fake_activation_cache.uid, fake_tag.uid, ISO15693_3_UID_SIZE);
    fake_activation_cache.system_info.flags = ISO15693_3_SYSINFO_FLAG_MEMORY;
    fake_activation_cache.system_info.block_count = fake_tag.advertised;
    fake_activation_cache.system_info.block_size = fake_tag.block_size;

    // Stop at the first block that does not answer, leaving the rest zeroed. This is the SDK behaviour
    // iso15693_poller.c's read-back note describes: activation passes its read through
    // iso15693_3_poller_filter_error, which maps Timeout and NotSupported to None, so activation can
    // report success having stopped reading early.
    for(uint16_t b = 0; b < fake_tag.advertised && b < FAKE_MAX_BLOCKS; b++) {
        if(fake_tag.kind[b] == FakeBlockAbsent) break;
        memcpy(
            fake_activation_cache.block_data + (size_t)b * FAKE_MAX_BLOCK_SIZE,
            fake_tag.content[b],
            fake_tag.block_size);
    }
}

void fake_tag_cache_all_advertised(void) {
    fake_tag_cache_from_activation();
    for(uint16_t b = 0; b < fake_tag.advertised && b < FAKE_MAX_BLOCKS; b++) {
        memcpy(
            fake_activation_cache.block_data + (size_t)b * FAKE_MAX_BLOCK_SIZE,
            fake_tag.content[b],
            fake_tag.block_size);
    }
}

void fake_data_init(Iso15693_3Data* data, uint16_t blocks, uint8_t block_size) {
    memset(data, 0, sizeof(*data));
    data->system_info.flags = ISO15693_3_SYSINFO_FLAG_MEMORY;
    data->system_info.block_count = blocks;
    data->system_info.block_size = block_size;
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; i++) {
        data->uid[i] = (uint8_t)(0xE0 + i);
    }
    if(blocks > 0) fake_data_fill(data, 0, (uint16_t)(blocks - 1), FAKE_MARKER);
}

void fake_data_fill(Iso15693_3Data* data, uint16_t first, uint16_t last, uint8_t byte) {
    const uint8_t size = data->system_info.block_size > FAKE_MAX_BLOCK_SIZE ?
                             (uint8_t)FAKE_MAX_BLOCK_SIZE :
                             data->system_info.block_size;
    for(uint16_t b = first; b <= last && b < FAKE_MAX_BLOCKS; b++) {
        memset(data->block_data + (size_t)b * FAKE_MAX_BLOCK_SIZE, byte, size);
    }
}

// ---- the SDK surface the poller calls -------------------------------------------------------------

Iso15693_3Error iso15693_3_poller_inventory(Iso15693_3Poller* instance, uint8_t* uid) {
    (void)instance;
    fake_charge_op();
    fake_tag.inventories++;
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) {
        return Iso15693_3ErrorTimeout;
    }
    memcpy(uid, fake_tag.uid, ISO15693_3_UID_SIZE);
    return Iso15693_3ErrorNone;
}

Iso15693_3Error iso15693_3_poller_read_block(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint8_t block_number,
    uint8_t block_size) {
    (void)instance;
    fake_charge_op();
    fake_tag.reads_attempted++;
    if(!fake_block_answers(block_number)) return Iso15693_3ErrorTimeout;
    memcpy(data, fake_tag.content[block_number], block_size);
    return Iso15693_3ErrorNone;
}

Iso15693_3Error iso15693_3_poller_write_block(
    Iso15693_3Poller* instance,
    const uint8_t* data,
    uint8_t block_number,
    uint8_t block_size) {
    (void)instance;
    fake_charge_op();
    fake_tag.writes_attempted++;
    // A lifted card answers nothing at all -> the radio layer times out.
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) {
        return Iso15693_3ErrorTimeout;
    }
    // Past-capacity blocks measured on hardware 2026-08-04 REFUSE the write in-band while failing the
    // read outright, so a write above the card's top returns Internal, not silence. Both are non-None
    // and the sweep treats them alike, but matching the measurement keeps the model honest.
    if(fake_tag.kind[block_number] == FakeBlockAbsent) return Iso15693_3ErrorInternal;
    // A present-but-locked block answers with an in-band error too.
    if(fake_tag.kind[block_number] == FakeBlockLocked) return Iso15693_3ErrorInternal;
    memcpy(fake_tag.content[block_number], data, block_size);
    fake_tag.writes_accepted++;
    return Iso15693_3ErrorNone;
}

Iso15693_3Error
    iso15693_3_poller_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data) {
    (void)instance;
    fake_charge_op();
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) {
        return Iso15693_3ErrorTimeout;
    }
    data->flags = ISO15693_3_SYSINFO_FLAG_MEMORY;
    data->block_count = fake_tag.advertised;
    data->block_size = fake_tag.block_size;
    return Iso15693_3ErrorNone;
}

Iso15693_3Error
    iso15693_3_poller_send_frame(Iso15693_3Poller* instance, void* tx, void* rx, uint32_t fwt) {
    (void)instance;
    (void)tx;
    (void)rx;
    (void)fwt;
    fake_charge_op();
    return Iso15693_3ErrorNone;
}

uint16_t iso15693_3_get_block_count(const Iso15693_3Data* data) {
    return data->system_info.block_count;
}

uint8_t iso15693_3_get_block_size(const Iso15693_3Data* data) {
    return data->system_info.block_size;
}

const uint8_t* iso15693_3_get_block_data(const Iso15693_3Data* data, uint8_t block) {
    // The SDK furi_check()s this; block_held_data's comment says its range check relies on that.
    furi_check(block < iso15693_3_get_block_count(data));
    return data->block_data + (size_t)block * FAKE_MAX_BLOCK_SIZE;
}

Iso15693_3Data* iso15693_3_alloc(void) {
    return calloc(1, sizeof(Iso15693_3Data));
}

void iso15693_3_free(Iso15693_3Data* data) {
    free(data);
}

void iso15693_3_copy(Iso15693_3Data* dest, const Iso15693_3Data* src) {
    memcpy(dest, src, sizeof(Iso15693_3Data));
}

void iso15693_3_reset(Iso15693_3Data* data) {
    memset(data, 0, sizeof(Iso15693_3Data));
}

const void* nfc_poller_get_data(NfcPoller* instance) {
    (void)instance;
    return &fake_activation_cache;
}

// Not exercised by the sweep tests; present so the whole translation unit links. Abort rather than
// no-op, so a test that unexpectedly reaches the radio-session machinery fails loudly.
NfcPoller* nfc_poller_alloc(Nfc* nfc, NfcProtocol protocol) {
    (void)nfc;
    (void)protocol;
    return NULL;
}
void nfc_poller_free(NfcPoller* instance) {
    (void)instance;
}
void nfc_poller_start(NfcPoller* instance, NfcGenericCallback callback, void* context) {
    (void)instance;
    (void)callback;
    (void)context;
    abort();
}
void nfc_poller_stop(NfcPoller* instance) {
    (void)instance;
}

BitBuffer* bit_buffer_alloc(size_t capacity) {
    (void)capacity;
    return NULL;
}
void bit_buffer_free(BitBuffer* buf) {
    (void)buf;
}
void bit_buffer_reset(BitBuffer* buf) {
    (void)buf;
}
void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte) {
    (void)buf;
    (void)byte;
}
