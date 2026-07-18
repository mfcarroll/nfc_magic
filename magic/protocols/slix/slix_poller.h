#pragma once

#include <nfc/nfc_poller.h>
#include "slix_data.h"

typedef struct SlixPoller SlixPoller;

typedef enum {
    SlixPollerModeInfo, // detect + read UID / system info
    SlixPollerModeWriteUid, // magic (gen1) backdoor UID write
} SlixPollerMode;

typedef enum {
    SlixPollerEventSuccess,
    SlixPollerEventFail,
    SlixPollerEventCardLost,
} SlixPollerEvent;

typedef enum {
    SlixPollerErrorNone,
    SlixPollerErrorNotDetected,
    SlixPollerErrorTimeout,
} SlixPollerError;

typedef void (*SlixPollerCallback)(SlixPollerEvent event, void* context);

SlixPoller* slix_poller_alloc(Nfc* nfc);

void slix_poller_free(SlixPoller* instance);

// Detect + read (Info mode).
void slix_poller_start(SlixPoller* instance, SlixPollerCallback callback, void* context);

// Magic UID write. `uid` is ISO15693_3_UID_SIZE bytes, MSB-first (uid[0] must be 0xE0).
// The poller writes the gen1 backdoor blocks and then reports Success only if a read-back
// inventory returns the requested UID.
void slix_poller_start_write_uid(
    SlixPoller* instance,
    const uint8_t* uid,
    SlixPollerCallback callback,
    void* context);

void slix_poller_stop(SlixPoller* instance);

SlixData* slix_poller_get_data(SlixPoller* instance);
