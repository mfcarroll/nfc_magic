#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char* slix_info_get_manufacturer_name(uint8_t vendor_id);
char* slix_info_get_chip_info(uint8_t vendor_id, uint8_t chip_id);

#ifdef __cplusplus
}
#endif