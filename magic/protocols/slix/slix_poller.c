#include "slix_poller.h"
#include <furi.h>
#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <toolbox/bit_buffer.h>

// ISO15693_3_FDT_WRITE_POLL_FC is defined by the Momentum-slix SDK fork. Provide a fallback so this
// FAP also builds against a stock SDK that only ships ISO15693_3_FDT_POLL_FC (271200 carrier cycles
// ~= 20ms, the value the fork uses for a WRITE frame's response timeout).
#ifndef ISO15693_3_FDT_WRITE_POLL_FC
#define ISO15693_3_FDT_WRITE_POLL_FC (271200U)
#endif

// Magic ISO15693 ("Chinese magic") backdoor UID write, ported from proxmark3
// SetTag15693Uid / SetTag15693Uid_v2 (armsrc/iso15693.c). Unaddressed frames are sent to
// hidden backdoor blocks; the CRC is appended by iso15693_3_poller_send_frame. Two card
// generations exist and the write tries gen2 then (only if untouched) gen1.
#define SLIX_MAGIC_FLAGS (0x02U) // high data rate, unaddressed (ISO15_REQ_DATARATE_HIGH)

// gen1: WRITE BLOCK (0x21) to backdoor blocks; 4 data bytes each.
#define SLIX_MAGIC_CMD_WRITE  (0x21U) // ISO15693 WRITE BLOCK

// Standard ISO15693 identity writes, used to make a clone match the source's AFI / DSFID.
#define SLIX_MAGIC_CMD_WRITE_AFI   (0x27U) // ISO15693 WRITE AFI
#define SLIX_MAGIC_CMD_WRITE_DSFID (0x29U) // ISO15693 WRITE DSFID
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

// Give up after this many consecutive activation failures so neither the detect popup nor the write
// popup can hang forever with no card. Each failed activation adds a ~100ms delay in the SDK poller,
// so this is roughly a 5-7 second timeout.
#define SLIX_POLLER_MAX_ACTIVATION_ERRORS (40U)

// The verify read-back runs right after an RF field power-cycle, so retry the inventory a few times:
// a card that is momentarily slow to answer must not be misreported as removed (a false CardLost on
// an otherwise-successful write).
#define SLIX_POLLER_VERIFY_ATTEMPTS (3U)
#define SLIX_POLLER_VERIFY_RETRY_MS (5U)

// Write-mode state machine. Each verify runs after a NfcCommandReset field power-cycle.
typedef enum {
    SlixWriteStateStart, // read the current UID, send gen2, request a field reset
    SlixWriteStateVerifyGen2, // verify gen2; if the UID is untouched, send gen1 + reset
    SlixWriteStateVerifyGen1, // verify gen1
} SlixWriteState;

struct SlixPoller {
    NfcPoller* poller;
    SlixData* data;
    SlixPollerMode mode;
    uint8_t target_uid[ISO15693_3_UID_SIZE];
    uint8_t original_uid[ISO15693_3_UID_SIZE]; // UID before the write, to gate the gen1 fallback
    SlixWriteState write_state;
    uint32_t activation_errors; // consecutive activation failures (no card) -> timeout
    // Clone mode: the source image (kept separate from `data` so start_internal's reset can't wipe
    // it) and per-block write results.
    Iso15693_3Data* clone_source;
    uint16_t clone_blocks_total; // blocks on the source image
    uint16_t clone_failed_count; // in-range blocks that errored on write (locked/protected)
    uint16_t clone_over_capacity; // source blocks past the target's capacity (couldn't fit)
    uint8_t clone_failed_bitmap[SLIX_POLLER_BLOCK_BITMAP_SIZE];
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

// The gen2 CFG block also programs what the card *reports* for system-info geometry / IC ref. For a
// clone these are the source's values (so the copy advertises the same chip identity); otherwise the
// fixed magic defaults.
static void slix_poller_send_backdoor_uid_gen2(
    Iso15693_3Poller* iso_poller,
    const uint8_t* uid,
    uint8_t cfg_maxblock,
    uint8_t cfg_blocksize,
    uint8_t cfg_icref) {
    BitBuffer* tx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);
    BitBuffer* rx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);

    slix_poller_build_gen2_frame(
        tx, SLIX_MAGIC_V2_BLK_CFG, cfg_maxblock, cfg_blocksize, cfg_icref, 0x00);
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

// Best-effort: make the clone match the source's AFI / DSFID via the standard ISO15693 WRITE AFI /
// WRITE DSFID commands (only for fields the source actually reported). Frames: 02 27 <afi> and
// 02 29 <dsfid> (+CRC). Failures are ignored -- these are identity extras, not the core clone.
static void slix_poller_write_identity(Iso15693_3Poller* iso_poller, const Iso15693_3Data* source) {
    const Iso15693_3SystemInfo* sys = &source->system_info;
    BitBuffer* tx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);
    BitBuffer* rx = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);

    if(sys->flags & ISO15693_3_SYSINFO_FLAG_DSFID) {
        bit_buffer_reset(tx);
        bit_buffer_append_byte(tx, SLIX_MAGIC_FLAGS);
        bit_buffer_append_byte(tx, SLIX_MAGIC_CMD_WRITE_DSFID);
        bit_buffer_append_byte(tx, sys->dsfid);
        iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);
    }
    if(sys->flags & ISO15693_3_SYSINFO_FLAG_AFI) {
        bit_buffer_reset(tx);
        bit_buffer_append_byte(tx, SLIX_MAGIC_FLAGS);
        bit_buffer_append_byte(tx, SLIX_MAGIC_CMD_WRITE_AFI);
        bit_buffer_append_byte(tx, sys->afi);
        iso15693_3_poller_send_frame(iso_poller, tx, rx, ISO15693_3_FDT_WRITE_POLL_FC);
    }

    bit_buffer_free(tx);
    bit_buffer_free(rx);
}

static void slix_poller_report(SlixPoller* instance, SlixPollerEvent event) {
    if(instance->callback) {
        instance->callback(event, instance->context);
    }
}

// Clone mode: write every writable data block from the source image with the standard ISO15693 WRITE
// BLOCK. Locked blocks are skipped (they'd reject the write); real write errors are counted into the
// failure bitmap for Partial reporting. Runs synchronously on the Nfc worker thread.
static void slix_poller_write_source_blocks(SlixPoller* instance, Iso15693_3Poller* iso_poller) {
    const Iso15693_3Data* source = instance->clone_source;
    const uint16_t source_count = iso15693_3_get_block_count(source);
    const uint8_t block_size = iso15693_3_get_block_size(source);

    instance->clone_blocks_total = source_count;
    instance->clone_failed_count = 0;
    instance->clone_over_capacity = 0;
    memset(instance->clone_failed_bitmap, 0, sizeof(instance->clone_failed_bitmap));

    if(source_count == 0 || block_size == 0) return;

    // Don't write past the target's own capacity. A source read from a card that over-reports its
    // block count (or a genuinely larger card) would otherwise fail the out-of-range tail. Cap at
    // the target's reported block count when it's known and smaller, and report the shortfall as
    // "over capacity" rather than a write failure.
    const Iso15693_3Data* target = nfc_poller_get_data(instance->poller);
    const uint16_t target_count = iso15693_3_get_block_count(target);
    uint16_t write_count = source_count;
    if(target_count > 0 && target_count < write_count) {
        // Count only NON-EMPTY blocks past the target's capacity as a real shortfall: empty tail
        // blocks read back as zeros on the smaller card anyway, so the clone still matches exactly.
        for(uint16_t block = target_count; block < source_count && block < 256; block++) {
            const uint8_t* block_data = iso15693_3_get_block_data(source, block);
            for(uint8_t i = 0; i < block_size; i++) {
                if(block_data[i] != 0) {
                    instance->clone_over_capacity++;
                    break;
                }
            }
        }
        write_count = target_count;
    }

    // block_number is a uint8_t on the wire, so 256 blocks is the ceiling.
    for(uint16_t block = 0; block < write_count && block < 256; block++) {
        if(iso15693_3_is_block_locked(source, block)) continue;
        const uint8_t* block_data = iso15693_3_get_block_data(source, block);
        Iso15693_3Error error =
            iso15693_3_poller_write_block(iso_poller, block_data, (uint8_t)block, block_size);
        if(error != Iso15693_3ErrorNone) {
            instance->clone_failed_count++;
            instance->clone_failed_bitmap[block / 8] |= (uint8_t)(1u << (block % 8));
        }
    }
}

// Wipe mode: write zeros to every writable data block on the card itself (UID untouched). Uses the
// target's own reported geometry and lock bits.
static void slix_poller_wipe_blocks(SlixPoller* instance, Iso15693_3Poller* iso_poller) {
    const Iso15693_3Data* target = nfc_poller_get_data(instance->poller);
    const uint16_t block_count = iso15693_3_get_block_count(target);
    const uint8_t block_size = iso15693_3_get_block_size(target);

    instance->clone_blocks_total = block_count;
    instance->clone_failed_count = 0;
    instance->clone_over_capacity = 0;
    memset(instance->clone_failed_bitmap, 0, sizeof(instance->clone_failed_bitmap));

    if(block_count == 0 || block_size == 0) return;

    uint8_t zeros[32] = {0};
    const uint8_t size = block_size > sizeof(zeros) ? (uint8_t)sizeof(zeros) : block_size;
    for(uint16_t block = 0; block < block_count && block < 256; block++) {
        if(iso15693_3_is_block_locked(target, block)) continue;
        Iso15693_3Error error =
            iso15693_3_poller_write_block(iso_poller, zeros, (uint8_t)block, size);
        if(error != Iso15693_3ErrorNone) {
            instance->clone_failed_count++;
            instance->clone_failed_bitmap[block / 8] |= (uint8_t)(1u << (block % 8));
        }
    }
}

// The terminal outcome once a write finishes: any block that failed to write, or a source that
// overran the target, makes it Partial; otherwise Success. (A bare UID write sets neither.)
static SlixPollerEvent slix_poller_success_or_partial(SlixPoller* instance) {
    if(instance->clone_failed_count > 0 || instance->clone_over_capacity > 0) {
        return SlixPollerEventPartial;
    }
    return SlixPollerEventSuccess;
}

// Read the UID back for verification, retrying a few times so a momentary miss right after the field
// power-cycle isn't mistaken for a removed card. Runs on the Nfc worker thread (furi_delay_ms is the
// same primitive the SDK poller uses between activation attempts).
static Iso15693_3Error slix_poller_verify_inventory(Iso15693_3Poller* iso_poller, uint8_t* uid) {
    Iso15693_3Error error = Iso15693_3ErrorNone;
    for(uint32_t attempt = 0; attempt < SLIX_POLLER_VERIFY_ATTEMPTS; attempt++) {
        error = iso15693_3_poller_inventory(iso_poller, uid);
        if(error == Iso15693_3ErrorNone) break;
        furi_delay_ms(SLIX_POLLER_VERIFY_RETRY_MS);
    }
    return error;
}

// Drives one write-mode step. Runs on the Nfc worker thread with the field active. Returns the
// NfcCommand for the poller: Reset power-cycles the field (so the next Ready verifies a freshly
// re-powered card), Stop ends the operation.
static NfcCommand slix_poller_write_step(SlixPoller* instance, Iso15693_3Poller* iso_poller) {
    uint8_t readback[ISO15693_3_UID_SIZE] = {0};

    switch(instance->write_state) {
    case SlixWriteStateStart: {
        // Wipe zeros the card's own blocks and never touches the UID, so it's a single pass with no
        // backdoor write or field reset.
        if(instance->mode == SlixPollerModeWipe) {
            slix_poller_wipe_blocks(instance, iso_poller);
            SlixPollerEvent outcome;
            if(instance->clone_blocks_total > 0 &&
               instance->clone_failed_count >= instance->clone_blocks_total) {
                outcome = SlixPollerEventFail; // nothing could be wiped (read-only / not writable)
            } else {
                outcome = slix_poller_success_or_partial(instance);
            }
            slix_poller_report(instance, outcome);
            return NfcCommandStop;
        }
        // Remember the current UID so the destructive gen1 fallback only runs if gen2 left the
        // card untouched. The poller read the UID into its data during activation.
        const Iso15693_3Data* poller_data = nfc_poller_get_data(instance->poller);
        memcpy(instance->original_uid, poller_data->uid, ISO15693_3_UID_SIZE);

        // The gen2 CFG block programs what the card reports for geometry / IC ref. For a clone, use
        // the source's values so the copy advertises the same chip identity; otherwise the fixed
        // magic default. (gen1 has no geometry block, so a gen1 fallback keeps the card's own.)
        uint8_t cfg_maxblock = SLIX_MAGIC_V2_CFG_MAXBLOCK;
        uint8_t cfg_blocksize = SLIX_MAGIC_V2_CFG_BLOCKSIZE;
        uint8_t cfg_icref = SLIX_MAGIC_V2_CFG_IC_REF;

        if(instance->mode == SlixPollerModeClone) {
            const Iso15693_3SystemInfo* sys = &instance->clone_source->system_info;
            if(sys->flags & ISO15693_3_SYSINFO_FLAG_MEMORY) {
                if(sys->block_count > 0) cfg_maxblock = (uint8_t)(sys->block_count - 1);
                if(sys->block_size > 0) cfg_blocksize = (uint8_t)(sys->block_size - 1);
            }
            if(sys->flags & ISO15693_3_SYSINFO_FLAG_IC_REF) cfg_icref = sys->ic_ref;

            // Match AFI/DSFID, then write the data blocks. The UID + geometry go last (below), so a
            // data-block write can't overwrite the UID/commit.
            slix_poller_write_identity(iso_poller, instance->clone_source);
            slix_poller_write_source_blocks(instance, iso_poller);
        }

        slix_poller_send_backdoor_uid_gen2(
            iso_poller, instance->target_uid, cfg_maxblock, cfg_blocksize, cfg_icref);
        instance->write_state = SlixWriteStateVerifyGen2;
        return NfcCommandReset;
    }

    case SlixWriteStateVerifyGen2: {
        if(slix_poller_verify_inventory(iso_poller, readback) != Iso15693_3ErrorNone) {
            slix_poller_report(instance, SlixPollerEventCardLost);
            return NfcCommandStop;
        }
        if(memcmp(readback, instance->target_uid, ISO15693_3_UID_SIZE) == 0) {
            slix_poller_report(instance, slix_poller_success_or_partial(instance));
            return NfcCommandStop;
        }
        if(memcmp(readback, instance->original_uid, ISO15693_3_UID_SIZE) == 0) {
            // gen2 changed nothing: a gen1 card, or a non-magic tag. Try the gen1 sequence. This is
            // a standard (destructive) WRITE BLOCK, so the write is gated behind a user confirm.
            slix_poller_send_backdoor_uid_gen1(iso_poller, instance->target_uid);
            instance->write_state = SlixWriteStateVerifyGen1;
            return NfcCommandReset;
        }
        // gen2 changed the UID but not to the target: stop rather than compound it with gen1.
        slix_poller_report(instance, SlixPollerEventFail);
        return NfcCommandStop;
    }

    case SlixWriteStateVerifyGen1:
    default: {
        if(slix_poller_verify_inventory(iso_poller, readback) != Iso15693_3ErrorNone) {
            slix_poller_report(instance, SlixPollerEventCardLost);
            return NfcCommandStop;
        }
        const bool ok = memcmp(readback, instance->target_uid, ISO15693_3_UID_SIZE) == 0;
        slix_poller_report(
            instance, ok ? slix_poller_success_or_partial(instance) : SlixPollerEventFail);
        return NfcCommandStop;
    }
    }
}

// Runs on the Nfc worker thread. Returns NfcCommand to control the poller.
static NfcCommand slix_poller_nfc_callback(NfcGenericEvent event, void* context) {
    SlixPoller* instance = context;
    furi_assert(instance);

    if(event.protocol != NfcProtocolIso15693_3) {
        return NfcCommandContinue;
    }

    Iso15693_3PollerEvent* iso_event = event.event_data;

    // Activation error => no card in the field (or removed). Retry a bounded number of times so the
    // popup can't hang forever, then report CardLost.
    if(iso_event->type == Iso15693_3PollerEventTypeError) {
        if(++instance->activation_errors >= SLIX_POLLER_MAX_ACTIVATION_ERRORS) {
            slix_poller_report(instance, SlixPollerEventCardLost);
            return NfcCommandStop;
        }
        return NfcCommandContinue;
    }

    if(iso_event->type != Iso15693_3PollerEventTypeReady) {
        return NfcCommandContinue;
    }

    instance->activation_errors = 0;

    if(instance->mode == SlixPollerModeInfo) {
        // The poller filled Iso15693_3Data (UID + system info) during activation.
        const Iso15693_3Data* poller_data = nfc_poller_get_data(instance->poller);
        iso15693_3_copy(instance->data->iso15693_3_data, poller_data);
        slix_poller_report(instance, SlixPollerEventSuccess);
        return NfcCommandStop;
    }

    // Write mode. event.instance is the concrete Iso15693_3Poller; raw frames must be sent there.
    return slix_poller_write_step(instance, event.instance);
}

SlixPoller* slix_poller_alloc(Nfc* nfc) {
    SlixPoller* instance = malloc(sizeof(SlixPoller));
    instance->poller = nfc_poller_alloc(nfc, NfcProtocolIso15693_3);
    instance->data = slix_data_alloc();
    instance->clone_source = iso15693_3_alloc();
    instance->mode = SlixPollerModeInfo;
    instance->write_state = SlixWriteStateStart;
    instance->activation_errors = 0;
    instance->clone_blocks_total = 0;
    instance->clone_failed_count = 0;
    instance->clone_over_capacity = 0;
    memset(instance->clone_failed_bitmap, 0, sizeof(instance->clone_failed_bitmap));
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
    iso15693_3_free(instance->clone_source);
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
    instance->write_state = SlixWriteStateStart;
    instance->activation_errors = 0;
    instance->clone_blocks_total = 0;
    instance->clone_failed_count = 0;
    instance->clone_over_capacity = 0;
    memset(instance->clone_failed_bitmap, 0, sizeof(instance->clone_failed_bitmap));
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

void slix_poller_start_clone(
    SlixPoller* instance,
    const Iso15693_3Data* source,
    SlixPollerCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(source);
    // Hold our own copy of the source so it survives the async write; target UID = the source's UID.
    iso15693_3_copy(instance->clone_source, source);
    memcpy(instance->target_uid, source->uid, ISO15693_3_UID_SIZE);
    slix_poller_start_internal(instance, SlixPollerModeClone, callback, context);
}

void slix_poller_start_wipe(SlixPoller* instance, SlixPollerCallback callback, void* context) {
    furi_assert(instance);
    slix_poller_start_internal(instance, SlixPollerModeWipe, callback, context);
}

void slix_poller_get_clone_result(
    SlixPoller* instance,
    uint16_t* blocks_total,
    uint16_t* failed_count,
    uint16_t* over_capacity,
    uint8_t* failed_bitmap) {
    furi_assert(instance);
    if(blocks_total) *blocks_total = instance->clone_blocks_total;
    if(failed_count) *failed_count = instance->clone_failed_count;
    if(over_capacity) *over_capacity = instance->clone_over_capacity;
    if(failed_bitmap) {
        memcpy(failed_bitmap, instance->clone_failed_bitmap, SLIX_POLLER_BLOCK_BITMAP_SIZE);
    }
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
