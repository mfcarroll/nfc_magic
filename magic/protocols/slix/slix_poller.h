#pragma once

#include <nfc/nfc_poller.h>
#include "slix_data.h"

typedef struct SlixPoller SlixPoller;

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

void slix_poller_start(SlixPoller* instance, SlixPollerCallback callback, void* context);

void slix_poller_stop(SlixPoller* instance);

SlixData* slix_poller_get_data(SlixPoller* instance);