// Host-side tests for file select's source check on an ISO15693 clone: which saved files it takes. The
// stock NFC app saves an NXP SLIX tag under SLIX, a protocol built on ISO15693-3, and a clone writes only
// the ISO15693-3 data underneath -- so such a save is a clone source, while a dump of another family is
// still the wrong card.
//
// Same technique as the other scene tests: the scene is compiled verbatim against the fakes, and where
// it navigates is recorded. The loaded device is stubbed here rather than in fake_write.c, because its
// protocol is exactly what each case varies.

#include "fake_scene.h"

#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

// ---- what file select reads that the shared fakes leave opaque -------------------------------------
// Completed here, not in fakes/, which stay minimal and are verified against the firmware: these exist
// only so the scene compiles. The enumerators are copied in order from lib/nfc/protocols (Momentum
// 87.15). The structs carry only the fields the scene reads, and no case here reaches the branches
// that read them.
enum {
    MfUltralightTypeOrigin,
    MfUltralightTypeNTAG203,
    MfUltralightTypeMfulC,
    MfUltralightTypeUL11,
    MfUltralightTypeUL21,
    MfUltralightTypeNTAG213,
    MfUltralightTypeNTAG215,
    MfUltralightTypeNTAG216,
    MfUltralightTypeNTAGI2C1K,
    MfUltralightTypeNTAGI2C2K,
    MfUltralightTypeNTAGI2CPlus1K,
    MfUltralightTypeNTAGI2CPlus2K,

    MfUltralightTypeNum,
};

typedef enum {
    MfClassicTypeMini,
    MfClassicType1k,
    MfClassicType4k,

    MfClassicTypeNum,
} MfClassicType;

typedef struct {
    uint8_t uid_len;
} Iso14443_3aData;

struct MfUltralightData {
    Iso14443_3aData* iso14443_3a_data;
    MfUltralightType type;
};

struct MfClassicData {
    Iso14443_3aData* iso14443_3a_data;
    MfClassicType type;
};

// The two SDK calls the shared fakes do not declare, declared as the SDK declares them.
const uint8_t* nfc_device_get_uid(const NfcDevice* instance, size_t* uid_len);
bool nfc_protocol_has_parent(NfcProtocol protocol, NfcProtocol parent_protocol);

#include "../../scenes/nfc_magic_scene_file_select.c" // NOLINT -- deliberate, see above

#include <stdio.h>
#include <string.h>

static int tests_run;
static int tests_failed;
static const char* current_test;
static bool current_failed;

#define CHECK(cond)                                                  \
    do {                                                             \
        if(!(cond)) {                                                \
            printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            current_failed = true;                                   \
        }                                                            \
    } while(0)

static void begin(const char* name) {
    current_test = name;
    current_failed = false;
    tests_run++;
}

static void end(void) {
    if(current_failed) {
        tests_failed++;
        printf("FAILED: %s\n", current_test);
    } else {
        printf("  ok  %s\n", current_test);
    }
}

// ---- the loaded file -------------------------------------------------------------------------------

static NfcMagicApp app;
static NfcProtocol loaded_protocol;
static const uint8_t loaded_uid[8] = {0xE0, 0x04, 0x01, 0x50, 0x20, 0x26, 0x06, 0x8C};

bool nfc_magic_load_from_file_select(NfcMagicApp* instance) {
    (void)instance;
    return true;
}

NfcProtocol nfc_device_get_protocol(const NfcDevice* instance) {
    (void)instance;
    return loaded_protocol;
}

const uint8_t* nfc_device_get_uid(const NfcDevice* instance, size_t* uid_len) {
    (void)instance;
    if(uid_len) *uid_len = sizeof(loaded_uid);
    return loaded_uid;
}

// Only the Classic and Ultralight branches read the data, and no case here takes them that far.
const NfcDeviceData* nfc_device_get_data(const NfcDevice* instance, NfcProtocol protocol) {
    (void)instance;
    (void)protocol;
    return NULL;
}

bool uscuid_ul_data_is_writable(const UscuidUlData* data) {
    (void)data;
    return false;
}

// The SDK's protocol tree, for the only relation that reaches ISO15693-3: SLIX is built on it
// (nfc_protocols[NfcProtocolSlix].parent_protocol in lib/nfc/protocols/nfc_protocol.c).
bool nfc_protocol_has_parent(NfcProtocol protocol, NfcProtocol parent_protocol) {
    return protocol == NfcProtocolSlix && parent_protocol == NfcProtocolIso15693_3;
}

// Select a file of `protocol` for an ISO15693 clone, and return the scene file select moved to.
static uint32_t select_for_iso15693_clone(NfcProtocol protocol) {
    memset(&app, 0, sizeof(app));
    app.protocol = NfcMagicProtocolIso15693;
    loaded_protocol = protocol;
    fake_scene_reset(0);
    nfc_magic_scene_file_select_on_enter(&app);
    CHECK(fake_scene.nav_count == 1);
    CHECK(fake_scene.navs[0].kind == FakeNavNext);
    return fake_scene.navs[0].scene_id;
}

// ---- the cases -------------------------------------------------------------------------------------

static void test_an_iso15693_3_dump_is_a_clone_source(void) {
    begin("an ISO15693-3 dump goes straight to the write");
    CHECK(select_for_iso15693_clone(NfcProtocolIso15693_3) == NfcMagicSceneWrite);
    end();
}

static void test_a_slix_save_from_the_stock_app_is_a_clone_source(void) {
    begin("a SLIX save from the stock NFC app goes straight to the write, not to the wrong-card screen");
    CHECK(select_for_iso15693_clone(NfcProtocolSlix) == NfcMagicSceneWrite);
    end();
}

static void test_a_dump_of_another_family_is_still_the_wrong_card(void) {
    begin("a MIFARE Classic dump is still the wrong card for an ISO15693 clone");
    CHECK(select_for_iso15693_clone(NfcProtocolMfClassic) == NfcMagicSceneWrongCard);
    end();
}

int main(void) {
    printf("iso15693 file select\n");
    test_an_iso15693_3_dump_is_a_clone_source();
    test_a_slix_save_from_the_stock_app_is_a_clone_source();
    test_a_dump_of_another_family_is_still_the_wrong_card();
    printf("%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
