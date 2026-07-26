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

    notification_message(instance->notifications, &sequence_error);

    const uint32_t reason =
        scene_manager_get_scene_state(instance->scene_manager, NfcMagicSceneSlixWriteFail);
    const bool card_lost = (reason == NfcMagicSlixWriteFailReasonCardLost);
    const char* message = card_lost ? "Card removed\nbefore the write\ncould finish." :
                                      "Not a magic tag.\nThis card doesn't\nsupport UID write.";

    widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
    widget_add_string_element(
        widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Write failed");
    widget_add_string_multiline_element(
        widget, 0, 13, AlignLeft, AlignTop, FontSecondary, message);

    // Only a lost card is worth retrying; a rejected backdoor write would just fail again.
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
