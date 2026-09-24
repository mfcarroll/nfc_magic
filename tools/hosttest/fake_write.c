// Extra stubs the write scene's translation unit needs in order to LINK, and nothing more.
//
// Every symbol here is referenced only from nfc_magic_scene_write_on_enter / _on_exit / _setup_view --
// poller lifecycle, popup text, the blink helpers, the icons. The routing under test lives in
// nfc_magic_scene_write_on_event, which reads plain fields off NfcMagicApp (protocol, iso15693_mode,
// iso15693_result) and calls the scene manager. It never reaches any of these.
//
// That is a deliberate boundary, and it is the honest one to draw: these tests assert which SCREEN a
// worker event routes to, not what a poller does to get there. The poller behaviour is covered against a
// fake tag in the five test_* files that include iso15693_poller.c. Stubbing a poller here would be
// wrong if the routing depended on one -- it does not, and if that ever changes, this file is where the
// compile will break and force the question.
//
// A stub that is never called cannot assert wrong behaviour. A stub that IS called silently can, so if
// one of these ever needs a body, give it a recorder in fake_scene.c instead of a lie here.

#include "fake_scene.h"

// The app header, not the individual poller headers: it establishes Nfc / NfcCommand / NfcProtocol
// first, which the poller headers assume a caller already has -- the same order the app itself uses.
#include "../../nfc_magic_app_i.h"

#include <gui/modules/popup.h>

// Icons: addresses only.
const Icon I_Loading_24;
const Icon I_NFC_manual_60x50;

// ---- popup -----------------------------------------------------------------------------------------

void popup_reset(Popup* popup) {
    UNUSED(popup);
}

void popup_set_header(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    UNUSED(popup);
    UNUSED(text);
    UNUSED(x);
    UNUSED(y);
    UNUSED(horizontal);
    UNUSED(vertical);
}

void popup_set_text(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    UNUSED(popup);
    UNUSED(text);
    UNUSED(x);
    UNUSED(y);
    UNUSED(horizontal);
    UNUSED(vertical);
}

void popup_set_icon(Popup* popup, uint8_t x, uint8_t y, const Icon* icon) {
    UNUSED(popup);
    UNUSED(x);
    UNUSED(y);
    UNUSED(icon);
}

// ---- app helpers -----------------------------------------------------------------------------------

void nfc_magic_app_blink_start(NfcMagicApp* instance) {
    UNUSED(instance);
}

void nfc_magic_app_blink_stop(NfcMagicApp* instance) {
    UNUSED(instance);
}

// ---- nfc device ------------------------------------------------------------------------------------

const NfcDeviceData* nfc_device_get_data(const NfcDevice* instance, NfcProtocol protocol) {
    UNUSED(instance);
    UNUSED(protocol);
    return NULL;
}

NfcProtocol nfc_device_get_protocol(const NfcDevice* instance) {
    UNUSED(instance);
    return NfcProtocolIso15693_3;
}

// ---- poller lifecycle ------------------------------------------------------------------------------
// Deliberately inert. Reached only from on_enter / on_exit.

Gen1aPoller* gen1a_poller_alloc(Nfc* nfc) {
    UNUSED(nfc);
    return NULL;
}
void gen1a_poller_free(Gen1aPoller* instance) {
    UNUSED(instance);
}
void gen1a_poller_start(Gen1aPoller* instance, Gen1aPollerCallback callback, void* context) {
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
void gen1a_poller_stop(Gen1aPoller* instance) {
    UNUSED(instance);
}

Gen2Poller* gen2_poller_alloc(Nfc* nfc) {
    UNUSED(nfc);
    return NULL;
}
void gen2_poller_free(Gen2Poller* instance) {
    UNUSED(instance);
}
void gen2_poller_start(Gen2Poller* instance, Gen2PollerCallback callback, void* context) {
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
void gen2_poller_stop(Gen2Poller* instance) {
    UNUSED(instance);
}

Gen4Poller* gen4_poller_alloc(Nfc* nfc) {
    UNUSED(nfc);
    return NULL;
}
void gen4_poller_free(Gen4Poller* instance) {
    UNUSED(instance);
}
void gen4_poller_set_password(Gen4Poller* instance, Gen4Password password) {
    UNUSED(instance);
    UNUSED(password);
}
void gen4_poller_start(Gen4Poller* instance, Gen4PollerCallback callback, void* context) {
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
void gen4_poller_stop(Gen4Poller* instance) {
    UNUSED(instance);
}

UscuidUlPoller* uscuid_ul_poller_alloc(Nfc* nfc) {
    UNUSED(nfc);
    return NULL;
}
void uscuid_ul_poller_free(UscuidUlPoller* instance) {
    UNUSED(instance);
}
void uscuid_ul_poller_set_password(UscuidUlPoller* instance, const uint8_t* password) {
    UNUSED(instance);
    UNUSED(password);
}
void uscuid_ul_poller_set_wakeup(UscuidUlPoller* instance, UscuidUlWakeup wakeup) {
    UNUSED(instance);
    UNUSED(wakeup);
}
void uscuid_ul_poller_start(
    UscuidUlPoller* instance,
    UscuidUlPollerCallback callback,
    void* context) {
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
void uscuid_ul_poller_stop(UscuidUlPoller* instance) {
    UNUSED(instance);
}

// Which ISO15693 entry point on_enter chose. The stubs are otherwise inert; this exists so a test can
// assert the BRANCH as well as the state on_enter leaves behind, which is the pair that matters when
// a flag is consumed rather than merely read.
const char* fake_iso15693_started = NULL;

Iso15693Poller* iso15693_poller_alloc(Nfc* nfc) {
    UNUSED(nfc);
    return NULL;
}
void iso15693_poller_free(Iso15693Poller* instance) {
    UNUSED(instance);
}
void iso15693_poller_stop(Iso15693Poller* instance) {
    UNUSED(instance);
}
void iso15693_poller_start_wipe(
    Iso15693Poller* instance,
    Iso15693PollerCallback callback,
    void* context) {
    fake_iso15693_started = "wipe";
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
void iso15693_poller_start_write_uid(
    Iso15693Poller* instance,
    const uint8_t* uid,
    Iso15693PollerCallback callback,
    void* context) {
    fake_iso15693_started = "write_uid";
    UNUSED(instance);
    UNUSED(uid);
    UNUSED(callback);
    UNUSED(context);
}
void iso15693_poller_start_write_uid_gen1(
    Iso15693Poller* instance,
    const uint8_t* uid,
    Iso15693PollerCallback callback,
    void* context) {
    fake_iso15693_started = "write_uid_gen1";
    UNUSED(instance);
    UNUSED(uid);
    UNUSED(callback);
    UNUSED(context);
}
void iso15693_poller_start_clone(
    Iso15693Poller* instance,
    const Iso15693_3Data* source,
    Iso15693PollerCallback callback,
    void* context) {
    fake_iso15693_started = "clone";
    UNUSED(instance);
    UNUSED(source);
    UNUSED(callback);
    UNUSED(context);
}
void iso15693_poller_start_clone_gen1(
    Iso15693Poller* instance,
    const Iso15693_3Data* source,
    Iso15693PollerCallback callback,
    void* context) {
    fake_iso15693_started = "clone_gen1";
    UNUSED(instance);
    UNUSED(source);
    UNUSED(callback);
    UNUSED(context);
}

// The scene's poller callback copies this into instance->iso15693_result. The routing tests set that
// field directly, so this is never the source of the data under test.
void iso15693_poller_get_result(Iso15693Poller* instance, Iso15693PollerResult* result) {
    UNUSED(instance);
    UNUSED(result);
}
