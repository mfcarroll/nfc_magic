#include "../nfc_magic_app_i.h"

void nfc_magic_scene_iso15693_write_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagicApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_magic_scene_iso15693_write_fail_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    const uint32_t reason =
        scene_manager_get_scene_state(instance->scene_manager, NfcMagicSceneIso15693WriteFail);
    const bool card_lost = (reason == NfcMagicIso15693WriteFailReasonCardLost);
    const bool partial = (reason == NfcMagicIso15693WriteFailReasonPartial);
    const bool over_capacity = (reason == NfcMagicIso15693WriteFailReasonOverCapacity);

    // Over-capacity is a clean success (nothing was lost) -> success tone. Partial means something
    // didn't write, and card-lost / not-magic are outright failures -> error tone.
    notification_message(
        instance->notifications, over_capacity ? &sequence_success : &sequence_error);

    if(over_capacity) {
        // Clean success: the copy matched the source, the card just advertises more blocks than it
        // physically holds (the extra source blocks were empty, so nothing was lost). Concise summary
        // here, gen2-style; the exact empty top blocks are behind "Details".
        const uint16_t advertised = instance->iso15693_clone_blocks_total;
        const uint16_t extra = instance->iso15693_clone_over_capacity;
        const uint16_t physical = (advertised > extra) ? (uint16_t)(advertised - extra) :
                                                         advertised;
        widget_add_string_element(
            widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Clone complete");
        FuriString* text = furi_string_alloc();
        furi_string_printf(
            text,
            "Copy matches source.\nHolds %u/%u blocks.\nTop %u empty.",
            physical,
            advertised,
            extra);
        widget_add_string_multiline_element(
            widget, 4, 20, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(text));
        furi_string_free(text);
    } else if(partial) {
        // Summary only -- counts here, the per-block list behind "Details" -- mirroring the Gen2 /
        // USCUID-UL partial screens. Partial means some blocks wouldn't write (real source data lost,
        // or empty failures that weren't a clean capacity tail), or a clone fell back to gen1.
        const uint16_t total = instance->iso15693_clone_blocks_total;
        // Everything that didn't write: real-data losses plus any empty blocks past capacity.
        const uint16_t not_written =
            instance->iso15693_clone_failed_count + instance->iso15693_clone_over_capacity;
        const uint16_t ok = (total >= not_written) ? (uint16_t)(total - not_written) : 0;
        widget_add_string_element(
            widget,
            64,
            0,
            AlignCenter,
            AlignTop,
            FontPrimary,
            instance->iso15693_is_wipe_mode ? "Wipe partial" : "Clone partial");
        FuriString* text = furi_string_alloc();
        furi_string_printf(
            text,
            instance->iso15693_is_wipe_mode ? "Wiped %u/%u blocks\nNot cleared: %u" :
                                              "Cloned %u/%u blocks\nNot written: %u",
            ok,
            total,
            not_written);
        if(instance->iso15693_clone_capacity_confirmed &&
           instance->iso15693_clone_failed_count > 0) {
            // Real data was lost because those blocks are a persistent, contiguous run at the top of
            // the card -> the card is physically smaller than the source. (An empty top tail loses
            // nothing and is reported as an over-capacity success, not here.)
            furi_string_cat_str(text, "\nCard too small");
        } else if(instance->iso15693_clone_used_gen1) {
            // gen1 fallback stamped the UID/commit into blocks 56/57/62/63, so they differ.
            furi_string_cat_str(text, "\ngen1: 56/57/62/63 differ");
        }
        widget_add_string_multiline_element(
            widget, 4, 20, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(text));
        furi_string_free(text);
    } else {
        const char* message = card_lost ?
                                  "Card removed\nbefore the write\ncould finish." :
                                  "Not a magic tag.\nThis card doesn't\nsupport UID write.";
        widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
        widget_add_string_element(
            widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Write failed");
        widget_add_string_multiline_element(
            widget, 0, 13, AlignLeft, AlignTop, FontSecondary, message);
    }

    if(card_lost) {
        // Card removed mid-write -> retryable. Retry re-runs the write; Exit leaves. Matches the
        // generic write-fail screen (Retry left, Exit right).
        widget_add_button_element(
            widget,
            GuiButtonTypeLeft,
            "Retry",
            nfc_magic_scene_iso15693_write_fail_widget_callback,
            instance);
        widget_add_button_element(
            widget,
            GuiButtonTypeRight,
            "Exit",
            nfc_magic_scene_iso15693_write_fail_widget_callback,
            instance);
    } else {
        // over-capacity / partial are (qualified) successes -> "Finish"; not-magic is a failure ->
        // "Back". The primary exit sits on the left, like the Gen2/USCUID/gen4 result screens.
        widget_add_button_element(
            widget,
            GuiButtonTypeLeft,
            (over_capacity || partial) ? "Finish" : "Back",
            nfc_magic_scene_iso15693_write_fail_widget_callback,
            instance);
        // "Details" (forward) lists the affected blocks, like Gen2 / USCUID: a partial's failed
        // blocks, or an over-capacity's empty top blocks.
        if(over_capacity || (partial && instance->iso15693_clone_failed_count > 0)) {
            widget_add_button_element(
                widget,
                GuiButtonTypeRight,
                "Details",
                nfc_magic_scene_iso15693_write_fail_widget_callback,
                instance);
        }
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_iso15693_write_fail_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    const uint32_t reason =
        scene_manager_get_scene_state(instance->scene_manager, NfcMagicSceneIso15693WriteFail);
    const bool card_lost = (reason == NfcMagicIso15693WriteFailReasonCardLost);
    const bool partial = (reason == NfcMagicIso15693WriteFailReasonPartial);
    const bool over_capacity = (reason == NfcMagicIso15693WriteFailReasonOverCapacity);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            if(card_lost) {
                // Retry: back to the write scene, which re-runs the write on enter.
                consumed = scene_manager_previous_scene(instance->scene_manager);
            } else {
                // Finish / Back -> the ISO15693 menu.
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcMagicSceneIso15693);
            }
        } else if(event.event == GuiButtonTypeRight) {
            if(partial || over_capacity) {
                // Details -> the affected-block list (failed blocks, or the empty top blocks).
                scene_manager_next_scene(
                    instance->scene_manager, NfcMagicSceneIso15693PartialDetails);
                consumed = true;
            } else {
                // Card-lost "Exit" -> the ISO15693 menu.
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcMagicSceneIso15693);
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcMagicSceneIso15693);
    }
    return consumed;
}

void nfc_magic_scene_iso15693_write_fail_on_exit(void* context) {
    NfcMagicApp* instance = context;

    // NOTE: do not reset the scene state here. The write scene always sets the reason before entering
    // this scene, and the "Details" round-trip re-enters this scene, which must re-read the same
    // reason -- clearing it here would rebuild the wrong screen on return from Details.
    widget_reset(instance->widget);
}
