#pragma once

#include <nfc/nfc_poller.h>
#include "slix_data.h"

typedef struct SlixPoller SlixPoller;

typedef enum {
    SlixPollerModeInfo, // detect + read UID / system info
    SlixPollerModeWriteUid, // magic backdoor UID write (gen2 first, then gen1 if untouched)
} SlixPollerMode;

typedef enum {
    SlixPollerEventSuccess, // info read ok, or the write verified against the target UID
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

void slix_poller_stop(SlixPoller* instance);

SlixData* slix_poller_get_data(SlixPoller* instance);
