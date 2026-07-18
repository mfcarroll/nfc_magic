#pragma once

#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>

typedef struct {
    Iso15693_3Data* iso15693_3_data;
    Iso15693_3SystemInfo system_info;
    bool system_info_ok;
} SlixData;

SlixData* slix_data_alloc();

void slix_data_free(SlixData* instance);

void slix_data_reset(SlixData* instance);

void slix_data_copy(SlixData* target, const SlixData* source);