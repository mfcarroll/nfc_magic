#include "../nfc_magic_app_i.h"

void nfc_magic_scene_slix_write_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagicApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_slix_write_fail_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    const uint32_t reason =
        scene_manager_get_scene_state(instance->scene_manager, NfcMagicSceneSlixWriteFail);
    const bool card_lost = (reason == NfcMagicSlixWriteFailReasonCardLost);
    const bool partial = (reason == NfcMagicSlixWriteFailReasonPartial);

    // Partial is a soft outcome (UID cloned, some data blocks didn't take); the rest are errors.
    notification_message(instance->notifications, partial ? &sequence_success : &sequence_error);

    if(partial) {
        // Full-width text box (no icon) so the detail can wrap. Partial means some NON-EMPTY source
        // blocks wouldn't write -- that data is past the card's real capacity (or the blocks are
        // locked), so it couldn't be cloned. Name the blocks. (Empty blocks that don't fit lose
        // nothing and never reach here -- they stay a clean Success.)
        FuriString* text = furi_string_alloc();
        furi_string_cat_str(text, instance->slix_is_wipe_mode ? "Wiped.\n" : "UID + data cloned.\n");
        if(instance->slix_clone_failed_count > 0) {
            furi_string_cat_printf(
                text,
                instance->slix_is_wipe_mode ? "%u block(s) wouldn't clear: " :
                                              "%u block(s) didn't fit the card: ",
                instance->slix_clone_failed_count);
            uint16_t shown = 0;
            for(uint16_t block = 0; block < 256; block++) {
                if(instance->slix_clone_failed_bitmap[block / 8] & (1u << (block % 8))) {
                    if(shown >= 20) {
                        furi_string_cat_str(text, "...");
                        break;
                    }
                    furi_string_cat_printf(text, "%u ", block);
                    shown++;
                }
            }
            furi_string_push_back(text, '\n');
        }
        if(instance->slix_clone_used_gen1) {
            // The clone fell back to the gen1 method, which stamps the UID/commit into blocks
            // 56/57/62/63 -- so those no longer match the source. (gen1 is not hardware-validated.)
            furi_string_cat_str(
                text, "gen1 method: blocks 56/57/62/63 hold the UID, not your file's data.");
        }
        widget_add_string_element(
            widget,
            3,
            0,
            AlignLeft,
            AlignTop,
            FontPrimary,
            instance->slix_is_wipe_mode ? "Wipe partial" : "Clone partial");
        widget_add_text_box_element(
            widget, 0, 14, 128, 38, AlignLeft, AlignTop, furi_string_get_cstr(text), false);
        furi_string_free(text);
    } else {
        const char* message = card_lost ? "Card removed\nbefore the write\ncould finish." :
                                          "Not a magic tag.\nThis card doesn't\nsupport UID write.";
        widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
        widget_add_string_element(
            widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Write failed");
        widget_add_string_multiline_element(
            widget, 0, 13, AlignLeft, AlignTop, FontSecondary, message);
    }

    // Only a lost card is worth retrying; a rejected/partial write would just repeat.
    if(card_lost) {
        widget_add_button_element(
            widget,
            GuiButtonTypeLeft,
            "Retry",
            nfc_magic_scene_slix_write_fail_widget_callback,
            instance);
    }
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "OK",
        nfc_magic_scene_slix_write_fail_widget_callback,
        instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_slix_write_fail_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            // Retry: back to the write scene, which re-runs the write on enter.
            consumed = scene_manager_previous_scene(instance->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcMagicSceneSlix);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcMagicSceneSlix);
    }
    return consumed;
}

void nfc_magic_scene_slix_write_fail_on_exit(void* context) {
    NfcMagicApp* instance = context;

    // Reset to the default reason so a later failure isn't mislabelled.
    scene_manager_set_scene_state(
        instance->scene_manager, NfcMagicSceneSlixWriteFail, NfcMagicSlixWriteFailReasonNotMagic);
    widget_reset(instance->widget);
}
