#pragma once

#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>

// Thin wrapper around the SDK's Iso15693_3Data. The ISO15693-3 poller already fills in
// system_info (block count/size, DSFID, AFI, IC ref) and the block data during activation,
// so there is nothing to duplicate here -- everything the SLIX scenes need lives inside
// iso15693_3_data.
typedef struct {
    Iso15693_3Data* iso15693_3_data;
} SlixData;

SlixData* slix_data_alloc();

void slix_data_free(SlixData* instance);

void slix_data_reset(SlixData* instance);

void slix_data_copy(SlixData* target, const SlixData* source);
