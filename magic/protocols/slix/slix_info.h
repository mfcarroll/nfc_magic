#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char* slix_info_get_manufacturer_name(uint8_t vendor_id);
char* slix_info_get_chip_info(uint8_t vendor_id, uint8_t chip_id);

// Richer chip decode that inspects the whole UID (MSB-first, uid[0]==0xE0). For NXP I-Code parts
// it distinguishes SLI / SLIX / SLIX2 (and the -S / -L variants) via the type-indicator bits of
// uid[3]; for everything else it falls back to slix_info_get_chip_info(uid[1], uid[2]).
char* slix_info_get_chip_info_ex(const uint8_t* uid);

#ifdef __cplusplus
}
#endif