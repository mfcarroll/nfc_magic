#include "../nfc_magic_app_i.h"
#include "../magic/protocols/iso15693/iso15693_poller.h"

// Shown mid-clone when the gen2 backdoor left the UID unchanged (not a gen2 magic card, or not magic
// at all). Offers the destructive, NOT-hardware-tested gen1 fallback as an explicit opt-in. Nothing
// has been written to the card yet, so declining leaves it untouched; accepting re-runs the clone in
// gen1 mode (the write scene reads iso15693_force_gen1 on enter).
static void nfc_magic_scene_iso15693_gen1_optin_button_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagicApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_iso15693_gen1_optin_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    widget_add_string_element(
        widget, 3, 0, AlignLeft, AlignTop, FontPrimary, "Not gen2 magic card");

    FuriString* body = furi_string_alloc();
    furi_string_cat_str(
        body,
        "Might be gen1, or not magic at all. Gen1 writes the UID to blocks 56/57/62/63 first, then "
        "the rest of the data only if that UID takes. A non-magic tag loses at most those 4 blocks. "
        "Gen1 is not hardware-tested.");
    // If the source itself stores data in those backdoor blocks, gen1 can't reproduce it -- warn at
    // the decision point (this is the source-side pre-check that used to sit on the up-front confirm).
    const Iso15693_3Data* source =
        nfc_device_get_data(instance->source_dev, NfcProtocolIso15693_3);
    if(iso15693_poller_source_uses_gen1_blocks(source)) {
        furi_string_cat_str(body, "\n\nYour file's data in 56/57/62/63 can't be cloned by gen1.");
    }
    // Scrolling body (Up/Down) so the full warning always fits alongside the button row.
    widget_add_text_scroll_element(widget, 0, 14, 128, 37, furi_string_get_cstr(body));
    furi_string_free(body);

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        nfc_magic_scene_iso15693_gen1_optin_button_callback,
        instance);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Try gen1",
        nfc_magic_scene_iso15693_gen1_optin_button_callback,
        instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_iso15693_gen1_optin_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            // Opt in: re-run the clone in gen1 mode. The write scene reads iso15693_force_gen1 on
            // enter; the gen2 attempt wrote nothing, so this is the first thing to touch the card.
            instance->iso15693_force_gen1 = true;
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWrite);
            consumed = true;
        } else if(event.event == GuiButtonTypeLeft) {
            // Decline: leave the card untouched, back to the ISO15693 menu.
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcMagicSceneIso15693);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcMagicSceneIso15693);
    }
    return consumed;
}

void nfc_magic_scene_iso15693_gen1_optin_on_exit(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);
}
