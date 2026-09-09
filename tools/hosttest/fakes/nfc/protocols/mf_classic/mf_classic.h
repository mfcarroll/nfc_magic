#pragma once
// Host-test stub. The code under test never calls into this module, but since the key-cache work
// landed upstream (#258/#261) its TYPES appear in struct fields the tests compile through --
// nfc_magic_app_i.h, gen2_poller.h and helpers/mfc_key_cache.h. Sizes and enum order must match
// the real header or a struct laid out here differs from the shipped one.
//
// Verified against unleashed-firmware/lib/nfc/protocols/mf_classic/mf_classic.h @ 3c9be0fd:
//   MF_CLASSIC_KEY_SIZE  :31  (6)
//   MfClassicKeyType     :85-88  (A, B -- in that order)
//   MfClassicKey         :90-92  (uint8_t data[MF_CLASSIC_KEY_SIZE])
#include <furi.h>

typedef struct MfClassicData MfClassicData;

#define MF_CLASSIC_KEY_SIZE (6)

typedef enum {
    MfClassicKeyTypeA,
    MfClassicKeyTypeB,
} MfClassicKeyType;

typedef struct {
    uint8_t data[MF_CLASSIC_KEY_SIZE];
} MfClassicKey;
