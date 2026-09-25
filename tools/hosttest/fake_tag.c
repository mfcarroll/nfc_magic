#include "fake_tag.h"

#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <toolbox/bit_buffer.h>
#include <stdarg.h>

uint32_t fake_tick = 0;
FakeTag fake_tag;
Iso15693_3Data fake_activation_cache;

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

Iso15693_3Error
    iso15693_3_poller_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data) {
    (void)instance;
    fake_charge_op();
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) {
        return Iso15693_3ErrorTimeout;
    }
    if(fake_tag.sysinfo_fails) return Iso15693_3ErrorTimeout;
    data->flags = ISO15693_3_SYSINFO_FLAG_MEMORY;
    // Advertising a field and holding a value are independent, deliberately: write_identity requires
    // both, so a fake that coupled them could never exercise the half that matters.
    if(fake_tag.advertises_dsfid) data->flags |= ISO15693_3_SYSINFO_FLAG_DSFID;
    if(fake_tag.advertises_afi) data->flags |= ISO15693_3_SYSINFO_FLAG_AFI;
    data->dsfid = fake_tag.dsfid;
    data->afi = fake_tag.afi;
    data->block_count = fake_tag.advertised;
    data->block_size = fake_tag.block_size;
    return Iso15693_3ErrorNone;
}

// The magic backdoor UID arrives as two frames carrying half the UID each, so hold them until both have
// landed. uid[0] is the MSB, and the block named 7654 carries uid[7..4] -- see the frame layout comments
// in iso15693_poller.c.
static uint8_t staged_uid[ISO15693_3_UID_SIZE];
static bool staged_low; // blocks named 7654 -> uid[7..4]
static bool staged_high; // blocks named 3210 -> uid[3..0]

static void fake_stage_uid_half(bool is_7654, const uint8_t* d) {
    if(is_7654) {
        staged_uid[7] = d[0];
        staged_uid[6] = d[1];
        staged_uid[5] = d[2];
        staged_uid[4] = d[3];
        staged_low = true;
    } else {
        staged_uid[3] = d[0];
        staged_uid[2] = d[1];
        staged_uid[1] = d[2];
        staged_uid[0] = d[3];
        staged_high = true;
    }
}

// Decode what the poller actually put on the wire. A tag that is not magic ignores all of it -- which is
// the case the gen2-then-gen1 flow exists to detect, so the fake has to be able to be that tag.
// Apply half a UID immediately, the way gen1 silicon does under the wipe's sweep: block 56 carries
// uid[7..4] and block 57 uid[3..0], each takes effect on its own, and an inventory in the same field
// session already returns the changed UID. Measured on NXP ICODE SLIX and ST LRi2K.
//
// Deliberately NOT the staging the gen1 BACKDOOR path below uses. That one holds both halves until a
// power-cycle, which is stricter than the hardware on purpose (see gen1_uid_pending in fake_tag.h);
// this one is the hazard itself -- the card's identity moving out from under a pass that is still
// running -- and deferring it would model the opposite of what was measured.
static void fake_apply_uid_half_now(bool is_7654, const uint8_t* d) {
    uint8_t uid[ISO15693_3_UID_SIZE];
    memcpy(uid, fake_tag.uid, sizeof(uid));
    if(is_7654) {
        uid[7] = d[0];
        uid[6] = d[1];
        uid[5] = d[2];
        uid[4] = d[3];
    } else {
        uid[3] = d[0];
        uid[2] = d[1];
        uid[1] = d[2];
        uid[0] = d[3];
    }
    fake_tag_set_uid_now(uid);
}

// Addressed WRITE BLOCK: 22 21 <uid, LSB first> <block> <data...>, or 62 21 ... with the OPTION flag.
// Every data-block write the app sends is this frame -- the clone's payload and the wipe's zeros alike.
static Iso15693_3Error fake_addressed_write(const BitBuffer* tx, BitBuffer* rx) {
    fake_tag.writes_attempted++;
    // A lifted card answers nothing at all -> the radio layer times out.
    if(fake_tag.ops_until_lifted && fake_tag.ops > fake_tag.ops_until_lifted) {
        return Iso15693_3ErrorTimeout;
    }

    // The UID travels least-significant byte first, so the frame's first address byte is uid[7].
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; i++) {
        if(tx->data[2 + i] != fake_tag.uid[ISO15693_3_UID_SIZE - 1 - i]) {
            // SILENCE, not an error frame. Measured on four of the five chips: a UID one byte wrong
            // gets no answer at all. It is what makes an addressed write worth sending, and it is
            // also exactly how a pass that failed to re-address after moving the UID looks.
            return Iso15693_3ErrorTimeout;
        }
    }

    const uint8_t block = tx->data[2 + ISO15693_3_UID_SIZE];
    const uint8_t* data = tx->data + 3 + ISO15693_3_UID_SIZE;
    bit_buffer_reset(rx);

    // Checked BEFORE the block rules, because it is a complaint about the frame rather than about the
    // block: the tag has not looked at the address it names yet.
    if(fake_tag.requires_option && (tx->data[0] & 0x40) == 0) {
        bit_buffer_append_byte(rx, ISO15693_3_RESP_FLAG_ERROR);
        bit_buffer_append_byte(rx, ISO15693_3_RESP_ERROR_OPTION);
        return Iso15693_3ErrorNone;
    }

    if(!fake_block_answers(block) || fake_tag.kind[block] == FakeBlockLocked) {
        // An IN-BAND refusal: a well-formed, CRC-valid error frame, which the radio layer reports as
        // a successful exchange. Only the response parse tells it from a write that took. Absent
        // blocks answer this way on hardware (measured 2026-08-04) rather than staying silent.
        bit_buffer_append_byte(rx, ISO15693_3_RESP_FLAG_ERROR);
        bit_buffer_append_byte(
            rx,
            fake_tag.kind[block] == FakeBlockLocked ? ISO15693_3_RESP_ERROR_BLOCK_LOCKED :
                                                      ISO15693_3_RESP_ERROR_BLOCK_UNAVAILABLE);
        return Iso15693_3ErrorNone;
    }

    if(fake_tag.kind[block] == FakeBlockSilentlyRefuses) return Iso15693_3ErrorTimeout;

    memcpy(fake_tag.content[block], data, fake_tag.block_size);
    fake_tag.writes_accepted++;
    if(fake_tag.is_gen1_magic && (block == 0x38 || block == 0x39)) {
        fake_apply_uid_half_now(block == 0x38, data);
    }
    if(fake_tag.writes_are_unacknowledged) {
        // Written, and nothing said about it. The radio layer reports this exactly as it reports an
        // absent card, which is the whole difficulty.
        return Iso15693_3ErrorTimeout;
    }
    bit_buffer_append_byte(rx, ISO15693_3_RESP_FLAG_NONE);
    return Iso15693_3ErrorNone;
}

Iso15693_3Error iso15693_3_poller_send_frame(
    Iso15693_3Poller* instance,
    const BitBuffer* tx,
    BitBuffer* rx,
    uint32_t fwt) {
    (void)instance;
    (void)fwt;
    fake_charge_op();

    const BitBuffer* buf = tx;
    if(buf == NULL || buf->size < 3) return Iso15693_3ErrorNone;

    // Addressed WRITE BLOCK: flags, command, 8 address bytes, block, and at least one data byte. The
    // OPTION bit is masked off before the match, so 0x22 and 0x62 both land here -- whether the tag
    // then MINDS which one it got is fake_tag.requires_option's business, below.
    if((buf->data[0] & ~ISO15693_3_REQ_FLAG_T4_OPTION) == 0x22 && buf->data[1] == 0x21 &&
       buf->size >= 12) {
        return fake_addressed_write(buf, rx);
    }

    if(buf->data[0] != 0x02) return Iso15693_3ErrorNone;

    // WRITE DSFID: 02 29 <value>.  WRITE AFI: 02 27 <value>.
    // Both return None whatever happens, which is the point: the SDK has no response parser for these,
    // so an in-band refusal is indistinguishable from success at this layer. Only the read-back tells
    // them apart, and that is what write_identity is built around.
    if((buf->data[1] == 0x29 || buf->data[1] == 0x27) && buf->size >= 3) {
        const bool is_dsfid = (buf->data[1] == 0x29);
        fake_tag.identity_writes_seen++;
        const bool transient_refusal = fake_tag.identity_writes_seen <=
                                       fake_tag.identity_writes_refused;
        const bool refused = transient_refusal ||
                             (is_dsfid ? fake_tag.refuses_dsfid : fake_tag.refuses_afi);
        if(!refused) {
            if(is_dsfid) {
                fake_tag.dsfid = buf->data[2];
                fake_tag.advertises_dsfid = true;
            } else {
                fake_tag.afi = buf->data[2];
                fake_tag.advertises_afi = true;
            }
        }
        return Iso15693_3ErrorNone;
    }

    // gen1: 02 21 <block> d0 d1 d2 d3
    if(buf->data[1] == 0x21 && buf->size >= 7) {
        const uint8_t block = buf->data[2];
        if(block == 0x38 || block == 0x39) {
            if(fake_tag.is_gen1_magic) fake_stage_uid_half(block == 0x38, &buf->data[3]);
            if(fake_tag.is_gen1_magic && staged_low && staged_high) {
                // Deferred to the power-cycle on purpose, not because hardware defers it; see
                // gen1_uid_pending in fake_tag.h.
                fake_tag_arm_gen1_uid(staged_uid);
                staged_low = staged_high = false;
            }
        }
        return Iso15693_3ErrorNone;
    }

    // gen2: 02 E0 09 <ref> d0 d1 d2 d3
    if(buf->data[1] == 0xE0 && buf->size >= 8 && buf->data[2] == 0x09) {
        const uint8_t ref = buf->data[3];
        if(ref == 0x40 || ref == 0x41) {
            if(fake_tag.is_gen2_magic) fake_stage_uid_half(ref == 0x40, &buf->data[4]);
            if(fake_tag.is_gen2_magic && staged_low && staged_high) {
                // The gen2 backdoor register space is separate from data blocks and takes effect at once.
                fake_tag_set_uid_now(staged_uid);
                staged_low = staged_high = false;
            }
        }
        return Iso15693_3ErrorNone;
    }

    return Iso15693_3ErrorNone;
}

void fake_tag_set_uid_now(const uint8_t* uid) {
    memcpy(fake_tag.uid, uid, ISO15693_3_UID_SIZE);
}

void fake_tag_arm_gen1_uid(const uint8_t* uid) {
    memcpy(fake_tag.gen1_pending_uid, uid, ISO15693_3_UID_SIZE);
    fake_tag.gen1_uid_pending = true;
}

void fake_tag_power_cycle(void) {
    if(fake_tag.gen1_uid_pending) {
        memcpy(fake_tag.uid, fake_tag.gen1_pending_uid, ISO15693_3_UID_SIZE);
        fake_tag.gen1_uid_pending = false;
    }
    // A fresh activation re-reads the card, so the cache is rebuilt from what answers NOW.
    fake_tag_cache_from_activation();
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
    return calloc(1, sizeof(BitBuffer));
}
void bit_buffer_free(BitBuffer* buf) {
    free(buf);
}
void bit_buffer_reset(BitBuffer* buf) {
    buf->size = 0;
}
void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte) {
    furi_check(buf->size < FAKE_FRAME_CAP);
    buf->data[buf->size++] = byte;
}
void bit_buffer_append_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes) {
    furi_check(buf->size + size_bytes <= FAKE_FRAME_CAP);
    memcpy(buf->data + buf->size, data, size_bytes);
    buf->size += size_bytes;
}
size_t bit_buffer_get_size_bytes(const BitBuffer* buf) {
    return buf->size;
}
uint8_t bit_buffer_get_byte(const BitBuffer* buf, size_t index) {
    furi_check(index < buf->size);
    return buf->data[index];
}
