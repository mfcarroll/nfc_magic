// Host-side stand-in for the SDK's nfc_poller.h, for tools/hosttest only.
#pragma once

#include <furi.h>

typedef struct Nfc Nfc;
typedef struct NfcPoller NfcPoller;

// NfcProtocol lives in its own header, as in the firmware, with the full enumerator list.
#include <nfc/protocols/nfc_protocol.h>

typedef enum {
    NfcCommandContinue,
    NfcCommandReset,
    NfcCommandStop,
} NfcCommand;

// Field-for-field the real one (lib/nfc/protocols/nfc_generic_event.h), minus the typedef indirection.
typedef struct {
    NfcProtocol protocol;
    void* instance;
    void* event_data;
} NfcGenericEvent;

typedef NfcCommand (*NfcGenericCallback)(NfcGenericEvent event, void* context);

NfcPoller* nfc_poller_alloc(Nfc* nfc, NfcProtocol protocol);
void nfc_poller_free(NfcPoller* instance);
void nfc_poller_start(NfcPoller* instance, NfcGenericCallback callback, void* context);
void nfc_poller_stop(NfcPoller* instance);

// Returns the data the poller read during ACTIVATION -- not a live view of the tag. That distinction is
// load-bearing: activation's block read stops at the first failure and leaves the rest zeroed, which is
// why iso15693_poller_block_held_data can only prove presence, never absence. fake_tag.c reproduces it.
const void* nfc_poller_get_data(NfcPoller* instance);
