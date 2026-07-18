#include "../nfc_magic_app_i.h"
#include "../magic/protocols/slix/slix_poller.h"

void nfc_magic_scene_slix_info_on_enter(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);

    SlixData* data = slix_poller_get_data(instance->slix_poller);
    const Iso15693_3Data* iso_data = data->iso15693_3_data;

    FuriString* temp_str = furi_string_alloc();

    furi_string_cat_str(temp_str, "SLIX Card Info\n");

    // Display UID
    furi_string_cat_str(temp_str, "UID:");
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
        furi_string_cat_printf(temp_str, " %02X", iso_data->uid[i]);
    }
    furi_string_push_back(temp_str, '\n');

    // Display System Info if available
    if(data->system_info_ok) {
        furi_string_cat_printf(
            temp_str, "Memory: %u blocks\n", data->system_info.block_count);
        furi_string_cat_printf(
            temp_str, "Block Size: %u bytes\n", data->system_info.block_size);
    } else {
        furi_string_cat_str(temp_str, "System Info not supported\n");
    }

    widget_add_text_box_element(
        instance->widget,
        0,
        0,
        128,
        64,
        AlignLeft,
        AlignTop,
        furi_string_get_cstr(temp_str),
        false);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_slix_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // Consume all events
}

void nfc_magic_scene_slix_info_on_exit(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);
}
