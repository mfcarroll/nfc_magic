#include "slix_poller.h"
#include <furi.h>
#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3_poller.h>

typedef enum {
    SlixPollerStateIdle,
    SlixPollerStateDetecting,
    SlixPollerStateStopped,
} SlixPollerState;

struct SlixPoller {
    NfcPoller* poller;
    SlixData* data;
    SlixPollerCallback callback;
    void* context;
    FuriThread* thread;
    SlixPollerState state;
};

// This is the callback passed to the low-level nfc_poller.
// It must return NfcCommand to control the poller's state.
static NfcCommand slix_poller_nfc_callback(NfcGenericEvent event, void* context) {
    SlixPoller* instance = context;
    furi_assert(instance);

    // We are only interested in ISO15693-3 events.
    if(event.protocol != NfcProtocolIso15693_3) {
        return NfcCommandContinue;
    }

    // The event_data for an ISO15693-3 poller is an Iso15693_3PollerEvent.
    Iso15693_3PollerEvent* iso_event = event.event_data;

    if(iso_event->type == Iso15693_3PollerEventTypeReady) {
        // The underlying poller has successfully activated the card.
        // The card's data, including system info, is now available.
        const Iso15693_3Data* poller_data = nfc_poller_get_data(instance->poller);

        // The poller has already populated the data structure it owns.
        // We need to copy that data into our own application-managed structure.
        // The `slix_data_copy` function is designed for `SlixData` to `SlixData` copies.
        // Here, we copy the underlying `Iso15693_3Data` from the poller into our `SlixData` wrapper.
        slix_data_copy(instance->data, (const SlixData*)poller_data);

        // Notify the high-level listener (the scene) of success.
        if(instance->callback) {
            instance->callback(SlixPollerEventSuccess, instance->context);
        }
        // Tell the poller to stop, as we have found what we're looking for.
        return NfcCommandStop;
    } else if(iso_event->type == Iso15693_3PollerEventTypeError) {
        // An error occurred during activation.
        if(instance->callback) {
            instance->callback(SlixPollerEventFail, instance->context);
        }
        return NfcCommandStop;
    }

    // For any other event type, just continue polling.
    return NfcCommandContinue;
}

// This thread runs the poller.
static int32_t slix_poller_thread(void* context) {
    SlixPoller* instance = context;

    // The nfc_poller_start function is blocking and runs the polling loop.
    // It will only return when its callback returns NfcCommandStop or
    // when nfc_poller_stop() is called from another thread.
    if(instance->state == SlixPollerStateDetecting) {
        slix_data_reset(instance->data);
        nfc_poller_start(instance->poller, slix_poller_nfc_callback, instance);
    }

    // The poller has stopped, so we can set our state to idle.
    instance->state = SlixPollerStateIdle;

    return 0;
}

SlixPoller* slix_poller_alloc(Nfc* nfc) {
    SlixPoller* instance = malloc(sizeof(SlixPoller));
    // Allocate a generic poller configured for the ISO15693-3 protocol.
    instance->poller = nfc_poller_alloc(nfc, NfcProtocolIso15693_3);
    instance->data = slix_data_alloc();
    instance->thread = furi_thread_alloc_ex("SlixPoller", 1024, slix_poller_thread, instance);
    instance->state = SlixPollerStateIdle;
    return instance;
}

void slix_poller_free(SlixPoller* instance) {
    furi_assert(instance);
    // Ensure the thread is stopped before freeing resources.
    if(instance->state != SlixPollerStateIdle) {
        slix_poller_stop(instance);
    }
    furi_thread_free(instance->thread);
    nfc_poller_free(instance->poller);
    slix_data_free(instance->data);
    free(instance);
}

void slix_poller_start(SlixPoller* instance, SlixPollerCallback callback, void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->context = context;
    instance->state = SlixPollerStateDetecting;
    furi_thread_start(instance->thread);
}

void slix_poller_stop(SlixPoller* instance) {
    furi_assert(instance);
    if(instance->state != SlixPollerStateIdle) {
        instance->state = SlixPollerStateStopped;
        // This call will interrupt the blocking nfc_poller_start() in the thread.
        nfc_poller_stop(instance->poller);
        // Wait for the thread to finish its execution.
        furi_thread_join(instance->thread);
    }
}

SlixData* slix_poller_get_data(SlixPoller* instance) {
    furi_assert(instance);
    return instance->data;
}
