#include "../nfc_magic_app_i.h"
#include "../magic/protocols/slix/slix_poller.h"

static void nfc_magic_scene_slix_write_poller_callback(SlixPollerEvent event, void* context) {
    NfcMagicApp* instance = context;

    if(event == SlixPollerEventSuccess) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventWorkerSuccess);
    } else {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventWorkerFail);
    }
}

void nfc_magic_scene_slix_write_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Popup* popup = instance->popup;

    popup_set_header(popup, "Writing UID", 68, 19, AlignCenter, AlignBottom);
    popup_set_text(popup, "Keep card on the\nback of Flipper", 68, 21, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewPopup);

    slix_poller_start_write_uid(
        instance->slix_poller,
        instance->slix_target_uid,
        nfc_magic_scene_slix_write_poller_callback,
        instance);
    nfc_magic_app_blink_start(instance);
}

bool nfc_magic_scene_slix_write_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicCustomEventWorkerSuccess) {
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneSuccess);
            consumed = true;
        } else if(event.event == NfcMagicCustomEventWorkerFail) {
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWriteFail);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_magic_scene_slix_write_on_exit(void* context) {
    NfcMagicApp* instance = context;
    slix_poller_stop(instance->slix_poller);
    nfc_magic_app_blink_stop(instance);
    popup_reset(instance->popup);
}
