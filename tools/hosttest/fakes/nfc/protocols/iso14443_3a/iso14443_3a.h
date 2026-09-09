#pragma once
// Host-test stub. No calls into this module, but ISO14443_3A_MAX_UID_SIZE now sizes array members
// the tests compile through -- nfc_magic_app_i.h's card_uid and nfc_magic_scanner.h -- since the
// key-cache work landed upstream (#258/#261). A wrong value silently changes those struct layouts.
//
// Verified against unleashed-firmware/lib/nfc/protocols/iso14443_3a/iso14443_3a.h @ 3c9be0fd:
//   ISO14443_3A_UID_10_BYTES :12  (10U)
//   ISO14443_3A_MAX_UID_SIZE :13  (= ISO14443_3A_UID_10_BYTES)
#include <furi.h>

#define ISO14443_3A_UID_4_BYTES  (4U)
#define ISO14443_3A_UID_7_BYTES  (7U)
#define ISO14443_3A_UID_10_BYTES (10U)
#define ISO14443_3A_MAX_UID_SIZE ISO14443_3A_UID_10_BYTES
