#include "../nfc_magic_app_i.h"
#include "../magic/protocols/slix/slix_poller.h"
#include "../magic/protocols/slix/slix_info.h"

void nfc_magic_scene_slix_info_on_enter(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);

    const Iso15693_3Data* iso_data = instance->slix_data->iso15693_3_data;
    const Iso15693_3SystemInfo* sys_info = &iso_data->system_info;

    FuriString* temp_str = furi_string_alloc();

    furi_string_cat_str(temp_str, "ISO15693 (SLIX)\n");

    // UID (stored MSB-first: uid[0] == 0xE0, uid[1] == manufacturer, uid[2] == IC id)
    furi_string_cat_str(temp_str, "UID:");
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
        furi_string_cat_printf(temp_str, " %02X", iso_data->uid[i]);
    }
    furi_string_push_back(temp_str, '\n');

    // Manufacturer + chip type, decoded from the UID.
    const uint8_t manufacturer_id = iso15693_3_get_manufacturer_id(iso_data);
    const uint8_t chip_id = iso_data->uid[2];
    furi_string_cat_printf(
        temp_str, "Mfr: %s\n", slix_info_get_manufacturer_name(manufacturer_id));
    furi_string_cat_printf(
        temp_str, "Chip: %s\n", slix_info_get_chip_info(manufacturer_id, chip_id));

    // Memory geometry from GET SYSTEM INFO (only valid when the flag bit is set).
    if(sys_info->flags & ISO15693_3_SYSINFO_FLAG_MEMORY) {
        furi_string_cat_printf(
            temp_str,
            "Memory: %u blocks x %u bytes\n",
            sys_info->block_count,
            sys_info->block_size);
    } else {
        furi_string_cat_str(temp_str, "Memory: not reported\n");
    }

    if(sys_info->flags & ISO15693_3_SYSINFO_FLAG_DSFID) {
        furi_string_cat_printf(temp_str, "DSFID: %02X\n", sys_info->dsfid);
    }
    if(sys_info->flags & ISO15693_3_SYSINFO_FLAG_AFI) {
        furi_string_cat_printf(temp_str, "AFI: %02X\n", sys_info->afi);
    }
    if(sys_info->flags & ISO15693_3_SYSINFO_FLAG_IC_REF) {
        furi_string_cat_printf(temp_str, "IC ref: %02X\n", sys_info->ic_ref);
    }

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_slix_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_magic_scene_slix_info_on_exit(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);
}
