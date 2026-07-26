#include "../nfc_magic_app_i.h"
#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>

void nfc_magic_scene_slix_write_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagicApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_slix_write_confirm_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_str(temp_str, "UID:");
    for(size_t i = 0; i < ISO15693_3_UID_SIZE; ++i) {
        furi_string_cat_printf(temp_str, " %02X", instance->slix_target_uid[i]);
    }
    furi_string_cat_str(
        temp_str, "\nMagic ISO15693 only. gen1 may\noverwrite data on a non-magic\ntag.");

    widget_add_string_element(widget, 3, 0, AlignLeft, AlignTop, FontPrimary, "Write UID?");
    widget_add_text_box_element(
        widget, 0, 13, 128, 38, AlignLeft, AlignTop, furi_string_get_cstr(temp_str), false);
    widget_add_button_element(
        widget,
        GuiButtonTypeCenter,
        "Write",
        nfc_magic_scene_slix_write_confirm_widget_callback,
        instance);
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        nfc_magic_scene_slix_write_confirm_widget_callback,
        instance);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_slix_write_confirm_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(instance->scene_manager);
        } else if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneSlixWrite);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_magic_scene_slix_write_confirm_on_exit(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);
}
