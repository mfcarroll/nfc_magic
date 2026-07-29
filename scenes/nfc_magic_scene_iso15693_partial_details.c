#include "../nfc_magic_app_i.h"
#include "nfc_magic_scene_partial_details_common.h"

// The per-block "which blocks didn't write/clear" list for an ISO15693 partial clone/wipe, reached
// via "Details" on the partial summary -- mirrors the Gen2 / USCUID-UL partial-details screens.
void nfc_magic_scene_iso15693_partial_details_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Widget* widget = instance->widget;

    // Reached from the write-fail summary (still on the stack): its reason picks the title. For an
    // over-capacity success these are the empty blocks the card can't physically hold (not a failure),
    // so soften the wording; for a partial they're the blocks that wouldn't write / clear.
    const uint32_t reason =
        scene_manager_get_scene_state(instance->scene_manager, NfcMagicSceneIso15693WriteFail);
    const bool over_capacity = (reason == NfcMagicIso15693WriteFailReasonOverCapacity);
    const char* title;
    if(over_capacity) {
        title = "Empty top blocks";
    } else {
        title = instance->iso15693_is_wipe_mode ? "Blocks not cleared" : "Blocks not written";
    }
    widget_add_string_element(widget, 0, 0, AlignLeft, AlignTop, FontPrimary, title);

    FuriString* message = furi_string_alloc();
    if(over_capacity) {
        // These empty blocks are past the card's physical capacity -- no data was lost, but say why
        // they weren't written. ("Card too small" is reserved for the partial screen, where real data
        // IS lost; this clone's data all fit.)
        furi_string_cat_str(message, "Didn't fit on the card:\n");
    }
    nfc_magic_partial_details_append_indices(
        message, instance->iso15693_clone_failed_bitmap, instance->iso15693_clone_blocks_total, 0);
    if(instance->iso15693_clone_used_gen1) {
        // The gen1 fallback stamped the UID/commit into blocks 56/57/62/63, so they differ from the
        // source regardless of the write results above.
        furi_string_cat_str(
            message, "\ngen1: 56/57/62/63 hold UID + unlock/commit, not file data.");
    }
    widget_add_text_scroll_element(widget, 0, 13, 128, 51, furi_string_get_cstr(message));
    furi_string_free(message);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewWidget);
}

bool nfc_magic_scene_iso15693_partial_details_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }
    return consumed;
}

void nfc_magic_scene_iso15693_partial_details_on_exit(void* context) {
    NfcMagicApp* instance = context;
    widget_reset(instance->widget);
}
