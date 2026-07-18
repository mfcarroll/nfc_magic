#include "slix_data.h"

#include <furi.h>

SlixData* slix_data_alloc() {
    SlixData* instance = malloc(sizeof(SlixData));
    instance->iso15693_3_data = iso15693_3_alloc();
    instance->system_info_ok = false;
    return instance;
}

void slix_data_free(SlixData* instance) {
    furi_assert(instance);
    iso15693_3_free(instance->iso15693_3_data);
    free(instance);
}

void slix_data_reset(SlixData* instance) {
    furi_assert(instance);
    iso15693_3_reset(instance->iso15693_3_data);
    instance->system_info_ok = false;
}

void slix_data_copy(SlixData* target, const SlixData* source) {
    furi_assert(target);
    furi_assert(source);
    iso15693_3_copy(target->iso15693_3_data, source->iso15693_3_data);
    memcpy(&target->system_info, &source->system_info, sizeof(source->system_info));
    target->system_info_ok = source->system_info_ok;
}