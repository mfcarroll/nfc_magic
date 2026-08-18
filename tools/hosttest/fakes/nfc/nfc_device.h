#pragma once
// Host-test stub: the code under test never calls into this module; only the type must exist.
#include <furi.h>
typedef struct NfcDevice NfcDevice;
typedef struct NfcDeviceData NfcDeviceData;

// The scene reads a Classic dump out of a loaded device. Only the protocol tag and the accessor are
// needed; the data itself is never inspected by the routing under test.
#include <nfc/protocols/nfc_protocol.h>

const NfcDeviceData* nfc_device_get_data(const NfcDevice* instance, NfcProtocol protocol);
NfcProtocol nfc_device_get_protocol(const NfcDevice* instance);
