#include "../nfc_magic_app_i.h"
#include "../magic/protocols/slix/slix_poller.h"

static void nfc_magic_slix_get_info_poller_callback(SlixPollerEvent event, void* context) {
    NfcMagicApp* instance = context;

    if(event == SlixPollerEventSuccess) {
        // On success, send a custom event to the scene manager to transition
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventSlixCardDetected);
    } else { // SlixPollerEventFail
        // On failure, send a different event to go back
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventSlixCardDetectFailed);
    }
}

void nfc_magic_scene_slix_get_info_on_enter(void* context) {
    NfcMagicApp* app = context;
    Popup* popup = app->popup;

    // Setup the popup view to instruct the user
    popup_set_header(popup, "Detecting ISO15693", 68, 19, AlignCenter, AlignBottom);
    popup_set_text(popup, "Approach card to the back of Flipper", 68, 21, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMagicAppViewPopup);

    // Allocate the poller here (not at app startup) so it doesn't hold the shared Nfc's
    // config across the scanner's run. Freed in on_exit.
    app->slix_poller = slix_poller_alloc(app->nfc);
    slix_poller_start(app->slix_poller, nfc_magic_slix_get_info_poller_callback, app);
    nfc_magic_app_blink_start(app);
}

bool nfc_magic_scene_slix_get_info_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicCustomEventSlixCardDetected) {
            // Keep the read result in the app so the next scene still has it after the
            // poller is freed in on_exit.
            slix_data_copy(app->slix_data, slix_poller_get_data(app->slix_poller));
            // Info shows the card; Save writes it to a .nfc file. Intent set by the SLIX menu.
            const uint32_t intent =
                scene_manager_get_scene_state(app->scene_manager, NfcMagicSceneSlixGetInfo);
            scene_manager_next_scene(
                app->scene_manager,
                (intent == NfcMagicSlixReadIntentSave) ? NfcMagicSceneSlixSaveName :
                                                         NfcMagicSceneSlixInfo);
            consumed = true;
        } else if(event.event == NfcMagicCustomEventSlixCardDetectFailed) {
            // Failed to detect, go back to the previous scene (the SLIX menu)
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, NfcMagicSceneSlix);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_magic_scene_slix_get_info_on_exit(void* context) {
    NfcMagicApp* app = context;

    slix_poller_stop(app->slix_poller);
    slix_poller_free(app->slix_poller);
    app->slix_poller = NULL;
    nfc_magic_app_blink_stop(app);

    // Reset the popup to a clean state for the next view
    popup_reset(app->popup);
}
