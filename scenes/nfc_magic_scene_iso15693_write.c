#include "../nfc_magic_app_i.h"
#include "../magic/protocols/iso15693/iso15693_poller.h"

static void
    nfc_magic_scene_iso15693_write_poller_callback(Iso15693PollerEvent event, void* context) {
    NfcMagicApp* instance = context;

    if(event == Iso15693PollerEventSuccess) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventWorkerSuccess);
    } else if(event == Iso15693PollerEventCardLost) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcMagicCustomEventCardLost);
    } else if(event == Iso15693PollerEventNotGen2) {
        // gen2 left the UID unchanged (not a gen2 magic card). Nothing has been written, so offer the
        // destructive gen1 attempt as an explicit opt-in -- same consent model as the clone.
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventIso15693NotGen2);
    } else if(event == Iso15693PollerEventFail) {
        // Card present but the backdoor write wasn't accepted (not magic), or gen1 was attempted and
        // its UID didn't take either.
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcMagicCustomEventWorkerFail);
    }
    // Any other event is ignored: this scene only drives a Write-UID, which emits nothing else.
}

void nfc_magic_scene_iso15693_write_on_enter(void* context) {
    NfcMagicApp* instance = context;
    Popup* popup = instance->popup;

    // Icon on the left; text right-aligned a few px in from the right edge (a small margin) so it
    // both clears the 60px-wide icon on the left and isn't jammed against the right edge.
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    popup_set_text(popup, "Writing UID\nKeep card\non the back", 122, 32, AlignRight, AlignCenter);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcMagicAppViewPopup);

    // Allocate the poller here (not at app startup); freed in on_exit.
    instance->iso15693_poller = iso15693_poller_alloc(instance->nfc);
    // Normally try gen2 only. If the user opted into the gen1 attempt on the "Not gen2 magic card"
    // screen (iso15693_force_gen1), run that instead -- this scene is re-entered for the retry.
    if(instance->iso15693_force_gen1) {
        iso15693_poller_start_write_uid_gen1(
            instance->iso15693_poller,
            instance->iso15693_target_uid,
            nfc_magic_scene_iso15693_write_poller_callback,
            instance);
    } else {
        iso15693_poller_start_write_uid(
            instance->iso15693_poller,
            instance->iso15693_target_uid,
            nfc_magic_scene_iso15693_write_poller_callback,
            instance);
    }
    nfc_magic_app_blink_start(instance);
}

bool nfc_magic_scene_iso15693_write_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicCustomEventWorkerSuccess) {
            // The poller verified the new UID reads back, so refresh the stored read result too --
            // otherwise re-entering Write UID (without a fresh Info read) would seed the byte editor
            // from the pre-write UID and look as if the write hadn't taken.
            memcpy(
                instance->iso15693_data->uid, instance->iso15693_target_uid, ISO15693_3_UID_SIZE);
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneSuccess);
            consumed = true;
        } else if(
            event.event == NfcMagicCustomEventWorkerFail ||
            event.event == NfcMagicCustomEventCardLost) {
            // ISO15693 has its own fail scene with a reason, instead of the generic write-fail: a
            // rejected backdoor write means "not a magic tag", not a transient error.
            const uint32_t reason = (event.event == NfcMagicCustomEventCardLost) ?
                                        NfcMagicIso15693WriteFailReasonCardLost :
                                        NfcMagicIso15693WriteFailReasonNotMagic;
            scene_manager_set_scene_state(
                instance->scene_manager, NfcMagicSceneIso15693WriteFail, reason);
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneIso15693WriteFail);
            consumed = true;
        } else if(event.event == NfcMagicCustomEventIso15693NotGen2) {
            // gen2 didn't take the UID -> offer the opt-in gen1 attempt. Tell the opt-in screen which
            // flow it came from: a Write-UID consents to the four UID registers only, not to a full
            // data-block write, and it returns here rather than to the clone's write scene.
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcMagicSceneIso15693Gen1Optin,
                NfcMagicIso15693Gen1OptinFromWriteUid);
            scene_manager_next_scene(instance->scene_manager, NfcMagicSceneIso15693Gen1Optin);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_magic_scene_iso15693_write_on_exit(void* context) {
    NfcMagicApp* instance = context;
    iso15693_poller_stop(instance->iso15693_poller);
    iso15693_poller_free(instance->iso15693_poller);
    instance->iso15693_poller = NULL;
    nfc_magic_app_blink_stop(instance);
    popup_reset(instance->popup);
}
