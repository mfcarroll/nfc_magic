#pragma once

#include <nfc/nfc_poller.h>
#include "slix_data.h"

typedef struct SlixPoller SlixPoller;

// Size (bytes) of the per-block failure bitmap; covers up to 256 blocks (the ISO15693 max).
#define SLIX_POLLER_BLOCK_BITMAP_SIZE (32U)

typedef enum {
    SlixPollerModeInfo, // detect + read UID / system info
    SlixPollerModeWriteUid, // magic backdoor UID write (gen2 first, then gen1 if untouched)
    SlixPollerModeClone, // write UID + all writable data blocks from a source image
} SlixPollerMode;

typedef enum {
    SlixPollerEventSuccess, // info read ok, or the write/clone verified against the target
    SlixPollerEventPartial, // clone: UID written, but some data blocks could not be written
    SlixPollerEventFail, // card present but the backdoor write was not accepted (not a magic tag)
    SlixPollerEventCardLost, // no card in the field / card removed before the operation finished
} SlixPollerEvent;

typedef void (*SlixPollerCallback)(SlixPollerEvent event, void* context);

SlixPoller* slix_poller_alloc(Nfc* nfc);

void slix_poller_free(SlixPoller* instance);

// Detect + read (Info mode). Emits Success once a card is read, or CardLost after a bounded number
// of activation attempts with no card in the field.
void slix_poller_start(SlixPoller* instance, SlixPollerCallback callback, void* context);

// Magic UID write. `uid` is ISO15693_3_UID_SIZE bytes, MSB-first (uid[0] must be 0xE0).
// The poller writes the gen2 backdoor sequence first (a harmless custom command on a non-magic tag)
// and, only if the gen2 write left the card's UID unchanged, falls back to the destructive gen1
// WRITE-BLOCK sequence. Before each read-back it power-cycles the field (like proxmark's
// switch_off + getUID) so a card that only latches the new UID after a reset is not misreported as a
// failure. Reports Success only if a read-back inventory returns the requested UID.
// See .notes/protocol-reference.md for the byte-level frames.
void slix_poller_start_write_uid(
    SlixPoller* instance,
    const uint8_t* uid,
    SlixPollerCallback callback,
    void* context);

// Full clone: write `source`'s UID (magic backdoor) and every writable data block (standard WRITE
// BLOCK, locked blocks skipped) onto a magic card. `source` is an ISO15693-3 image loaded from a
// saved .nfc. Reports Success (UID + all blocks), Partial (UID ok, some blocks failed), Fail (UID
// not accepted) or CardLost. Data blocks are written first, then the UID.
void slix_poller_start_clone(
    SlixPoller* instance,
    const Iso15693_3Data* source,
    SlixPollerCallback callback,
    void* context);

// After a clone, the per-block write result: total blocks, how many failed, and a bitmap (bit N =
// block N failed). Any out param may be NULL. `failed_bitmap` must hold SLIX_POLLER_BLOCK_BITMAP_SIZE
// bytes.
void slix_poller_get_clone_result(
    SlixPoller* instance,
    uint16_t* blocks_total,
    uint16_t* failed_count,
    uint8_t* failed_bitmap);

void slix_poller_stop(SlixPoller* instance);

SlixData* slix_poller_get_data(SlixPoller* instance);
