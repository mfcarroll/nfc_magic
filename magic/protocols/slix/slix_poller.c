#include "slix_poller.h"
#include <furi.h>
#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <toolbox/bit_buffer.h>

// Magic ISO15693 ("Chinese magic") backdoor UID write, ported from proxmark3
// SetTag15693Uid / SetTag15693Uid_v2 (armsrc/iso15693.c). Unaddressed frames are sent to
// hidden backdoor blocks; the CRC is appended by iso15693_3_poller_send_frame. Two card
// generations exist and the write tries gen1 then gen2 (the wrong generation is a no-op).
#define SLIX_MAGIC_FLAGS (0x02U) // high data rate, unaddressed (ISO15_REQ_DATARATE_HIGH)

// gen1: WRITE BLOCK (0x21) to backdoor blocks; 4 data bytes each.
#define SLIX_MAGIC_CMD_WRITE  (0x21U) // ISO15693 WRITE BLOCK
#define SLIX_MAGIC_BLK_UNLOCK (0x3EU) // written as 0
#define SLIX_MAGIC_BLK_COMMIT (0x3FU) // written as 0x6996 (arms the UID change)
#define SLIX_MAGIC_BLK_UID_LO (0x38U) // uid[7..4]
#define SLIX_MAGIC_BLK_UID_HI (0x39U) // uid[3..0]

// gen2: magic write command (0xE0) with a 0x09 subcommand and a block reference; 4 data
// bytes each. Frame layout: 02 E0 09 <ref> d0 d1 d2 d3 (+CRC).
#define SLIX_MAGIC_CMD_WRITE_V2  (0xE0U) // ISO15693_MAGIC_WRITE
#define SLIX_MAGIC_V2_SUB        (0x09U)
#define SLIX_MAGIC_V2_BLK_CFG    (0x47U) // system-info config: max block / block size / IC ref
#define SLIX_MAGIC_V2_BLK_CFG2   (0x52U) // written as 0
#define SLIX_MAGIC_V2_BLK_UID_HI (0x40U) // uid[7..4]
#define SLIX_MAGIC_V2_BLK_UID_LO (0x41U) // uid[3..0]
// Fixed config payload for the CFG block, verbatim from proxmark's gen2 sequence (matches a
// 64-block / 4-byte-block / IC-ref-0x8B card; these values are constant in proxmark too).
#define SLIX_MAGIC_V2_CFG_MAXBLOCK  (0x3FU)
#define SLIX_MAGIC_V2_CFG_BLOCKSIZE (0x03U)
#define SLIX_MAGIC_V2_CFG_IC_REF    (0x8BU)

#define SLIX_POLLER_BUF_SIZE (32U)

struct SlixPoller {
    NfcPoller* poller;
    SlixData* data;
    SlixPollerMode mode;
    uint8_t target_uid[ISO15693_3_UID_SIZE];
    SlixPollerCallback callback;
    void* context;
    bool running;
};

// gen1 frame: 02 21 <block> d0 d1 d2 d3 (+CRC).
static void slix_poller_build_gen1_frame(
    BitBuffer* tx,
    uint8_t block,
    uint8_t d0,
    uint8_t d1,
    uint8_t d2,
    uint8_t d3) {
    bit_buffer_reset(tx);
    bit_buffer_append_byte(tx, SLIX_MAGIC_FLAGS);
    bit_buffer_append_byte(tx, SLIX_MAGIC_CMD_WRITE);
    bit_buffer_append_byte(tx, block);
    bit_buffer_append_byte(tx, d0);
    bit_buffer_append_byte(tx, d1);
    bit_buffer_append_byte(tx, d2);
    bit_buffer_append_byte(tx, d3);
}

// gen2 frame: 02 E0 09 <ref> d0 d1 d2 d3 (+CRC).
static void slix_poller_build_gen2_frame(
    BitBuffer* tx,
    uint8_t ref,
    uint8_t d0,
    uint8_t d1,
    uint8_t d2,
    uint8_t d3) {
    bit_buffer_reset(tx);
    bit_buffer_append_byte(tx, SLIX_MAGIC_FLAGS);
    bit_buffer_append_byte(tx, SLIX_MAGIC_CMD_WRITE_V2);
    bit_buffer_append_byte(tx, SLIX_MAGIC_V2_SUB);
    bit_buffer_append_byte(tx, ref);
    bit_buffer_append_byte(tx, d0);
    bit_buffer_append_byte(tx, d1);
    bit_buffer_append_byte(tx, d2);
    bit_buffer_append_byte(tx, d3);
}

// Magic cards may not answer these writes, so per-frame transceive results are intentionally
// ignored; the UID read-back is the real check.
static void slix_poller_send_backdoor_uid_gen1(Iso15693_3Poller* iso_poller, const uint8_t* uid) {
    BitBuffer* tx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);
    BitBuffer* rx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);

    slix_poller_build_gen1_frame(tx, SLIX_MAGIC_BLK_UNLOCK, 0x00, 0x00, 0x00, 0x00);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen1_frame(tx, SLIX_MAGIC_BLK_COMMIT, 0x69, 0x96, 0x00, 0x00);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen1_frame(tx, SLIX_MAGIC_BLK_UID_LO, uid[7], uid[6], uid[5], uid[4]);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen1_frame(tx, SLIX_MAGIC_BLK_UID_HI, uid[3], uid[2], uid[1], uid[0]);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    bit_buffer_free(tx);
    bit_buffer_free(rx);
}

static void slix_poller_send_backdoor_uid_gen2(Iso15693_3Poller* iso_poller, const uint8_t* uid) {
    BitBuffer* tx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);
    BitBuffer* rx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);

    slix_poller_build_gen2_frame(
        tx,
        SLIX_MAGIC_V2_BLK_CFG,
        SLIX_MAGIC_V2_CFG_MAXBLOCK,
        SLIX_MAGIC_V2_CFG_BLOCKSIZE,
        SLIX_MAGIC_V2_CFG_IC_REF,
        0x00);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen2_frame(tx, SLIX_MAGIC_V2_BLK_CFG2, 0x00, 0x00, 0x00, 0x00);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen2_frame(tx, SLIX_MAGIC_V2_BLK_UID_HI, uid[7], uid[6], uid[5], uid[4]);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    slix_poller_build_gen2_frame(tx, SLIX_MAGIC_V2_BLK_UID_LO, uid[3], uid[2], uid[1], uid[0]);
    iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);

    bit_buffer_free(tx);
    bit_buffer_free(rx);
}

static bool slix_poller_verify_uid(Iso15693_3Poller* iso_poller, const uint8_t* expected_uid) {
    uint8_t readback[ISO15693_3_UID_SIZE] = {0};
    Iso15693_3Error error = iso15693_3_poller_inventory(iso_poller, readback);
    if(error != Iso15693_3ErrorNone) {
        return false;
    }
    return memcmp(readback, expected_uid, ISO15693_3_UID_SIZE) == 0;
}

// Runs on the Nfc worker thread. Returns NfcCommand to control the poller.
static NfcCommand slix_poller_nfc_callback(NfcGenericEvent event, void* context) {
    SlixPoller* instance = context;
    furi_assert(instance);

    if(event.protocol != NfcProtocolIso15693_3) {
        return NfcCommandContinue;
    }

    Iso15693_3PollerEvent* iso_event = event.event_data;

    if(iso_event->type == Iso15693_3PollerEventTypeReady) {
        if(instance->mode == SlixPollerModeWriteUid) {
            // event.instance is the concrete Iso15693_3Poller; raw frames must be sent here.
            Iso15693_3Poller* iso_poller = event.instance;
            // Try gen1 first; if the UID didn't take, try gen2. Sending the wrong
            // generation's frames to a card is a harmless no-op.
            slix_poller_send_backdoor_uid_gen1(iso_poller, instance->target_uid);
            bool ok = slix_poller_verify_uid(iso_poller, instance->target_uid);
            if(!ok) {
                slix_poller_send_backdoor_uid_gen2(iso_poller, instance->target_uid);
                ok = slix_poller_verify_uid(iso_poller, instance->target_uid);
            }
            if(instance->callback) {
                instance->callback(
                    ok ? SlixPollerEventSuccess : SlixPollerEventFail, instance->context);
            }
            return NfcCommandStop;
        }

        // Info mode: the poller filled Iso15693_3Data (UID + system info) during activation.
        const Iso15693_3Data* poller_data = nfc_poller_get_data(instance->poller);
        iso15693_3_copy(instance->data->iso15693_3_data, poller_data);
        if(instance->callback) {
            instance->callback(SlixPollerEventSuccess, instance->context);
        }
        return NfcCommandStop;
    }

    // Any other event (e.g. activation error because no card is in the field yet) just means
    // "keep polling" -- wait for a card to appear rather than bailing out. The owning scene
    // cancels by calling slix_poller_stop() on exit.
    return NfcCommandContinue;
}

SlixPoller* slix_poller_alloc(Nfc* nfc) {
    SlixPoller* instance = malloc(sizeof(SlixPoller));
    instance->poller = nfc_poller_alloc(nfc, NfcProtocolIso15693_3);
    instance->data = slix_data_alloc();
    instance->mode = SlixPollerModeInfo;
    instance->callback = NULL;
    instance->context = NULL;
    instance->running = false;
    return instance;
}

void slix_poller_free(SlixPoller* instance) {
    furi_assert(instance);
    if(instance->running) {
        slix_poller_stop(instance);
    }
    nfc_poller_free(instance->poller);
    slix_data_free(instance->data);
    free(instance);
}

// nfc_poller_start is non-blocking: the callback fires on the Nfc worker thread and returns
// NfcCommandStop when finished. The owning scene must still call slix_poller_stop() on exit
// so the NfcPoller session state is reset before the next start.
static void slix_poller_start_internal(
    SlixPoller* instance,
    SlixPollerMode mode,
    SlixPollerCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(!instance->running);
    instance->mode = mode;
    instance->callback = callback;
    instance->context = context;
    slix_data_reset(instance->data);
    instance->running = true;
    nfc_poller_start(instance->poller, slix_poller_nfc_callback, instance);
}

void slix_poller_start(SlixPoller* instance, SlixPollerCallback callback, void* context) {
    slix_poller_start_internal(instance, SlixPollerModeInfo, callback, context);
}

void slix_poller_start_write_uid(
    SlixPoller* instance,
    const uint8_t* uid,
    SlixPollerCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(uid);
    memcpy(instance->target_uid, uid, ISO15693_3_UID_SIZE);
    slix_poller_start_internal(instance, SlixPollerModeWriteUid, callback, context);
}

void slix_poller_stop(SlixPoller* instance) {
    furi_assert(instance);
    if(instance->running) {
        nfc_poller_stop(instance->poller);
        instance->running = false;
    }
}

SlixData* slix_poller_get_data(SlixPoller* instance) {
    furi_assert(instance);
    return instance->data;
}
