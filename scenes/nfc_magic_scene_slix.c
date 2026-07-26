#include "../nfc_magic_app_i.h"
#include "nfc_magic_scene.h"

enum SubmenuIndex {
    SubmenuIndexSlixInfo,
    SubmenuIndexSlixSave,
    SubmenuIndexSlixWriteUid,
};

void nfc_magic_scene_slix_submenu_callback(void* context, uint32_t index) {
    NfcMagicApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_magic_scene_slix_on_enter(void* context) {
    NfcMagicApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Info",
        SubmenuIndexSlixInfo,
        nfc_magic_scene_slix_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "Save to file",
        SubmenuIndexSlixSave,
        nfc_magic_scene_slix_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "Write UID",
        SubmenuIndexSlixWriteUid,
        nfc_magic_scene_slix_submenu_callback,
        app);

    submenu_set_header(submenu, "ISO15693 / NfcV");

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMagicAppViewMenu);
}

bool nfc_magic_scene_slix_on_event(void* context, SceneManagerEvent event) {
    NfcMagicApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSlixInfo || event.event == SubmenuIndexSlixSave) {
            // Both Info and Save read the card first; the read scene branches on this intent.
            const uint32_t intent = (event.event == SubmenuIndexSlixSave) ?
                                        NfcMagicSlixReadIntentSave :
                                        NfcMagicSlixReadIntentInfo;
            scene_manager_set_scene_state(
                app->scene_manager, NfcMagicSceneSlixGetInfo, intent);
            scene_manager_next_scene(app->scene_manager, NfcMagicSceneSlixGetInfo);
            consumed = true;
        } else if(event.event == SubmenuIndexSlixWriteUid) {
            scene_manager_next_scene(app->scene_manager, NfcMagicSceneSlixWriteInput);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, NfcMagicSceneStart);
    }

    return consumed;
}

void nfc_magic_scene_slix_on_exit(void* context) {
    NfcMagicApp* app = context;
    submenu_reset(app->submenu);
}