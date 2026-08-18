// Host-side tests for the write scene's ROUTING: which screen each worker event sends the user to, and
// which of the five magic protocols swallows Back.
//
// The shipped scene is compiled verbatim; the firmware calls resolve to the recorders in fake_scene.c and
// the link-only stubs in fake_write.c. What is asserted here is the mapping from (protocol, mode, result)
// to a destination screen and reason code -- a routing table expressed as nested branches, which is
// exactly the shape that goes wrong quietly.
//
// The centrepiece is the mode-gate added in round 5. `pass_truncated` used to be wipe-only; once a cut
// CLONE could set it, the branch that sends a truncated run to the wipe-specific "Wipe stopped" screen had
// to learn to ask which mode it was in. Nothing tested that until now -- it was the one piece of new
// behaviour in that round with no coverage at all.

#include "fake_scene.h"

#include "../../scenes/nfc_magic_scene_write.c" // NOLINT -- deliberate, see above

#include <stdio.h>

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

// ---- harness ---------------------------------------------------------------------------------------

static NfcMagicApp app;
static Iso15693_3Data iso_data;

static void setup(NfcMagicProtocol protocol, NfcMagicIso15693Mode mode) {
    memset(&app, 0, sizeof(app));
    memset(&iso_data, 0, sizeof(iso_data));
    app.protocol = protocol;
    app.iso15693_mode = mode;
    app.iso15693_data = &iso_data; // the Write-UID success path writes through this
    fake_scene_reset(0);
}

// Feed a custom event through on_event and report where it routed to.
static void send(uint32_t custom_event) {
    SceneManagerEvent ev = {.type = SceneManagerEventTypeCustom, .event = custom_event};
    nfc_magic_scene_write_on_event(&app, ev);
}

// The scene the last navigation targeted, or UINT32_MAX if it did not navigate.
static uint32_t routed_to(void) {
    for(int i = (int)fake_scene.nav_count - 1; i >= 0; i--) {
        if(fake_scene.navs[i].kind == FakeNavNext) return fake_scene.navs[i].scene_id;
    }
    return UINT32_MAX;
}

// The reason code the scene stashed for the result screen. set_scene_state writes through the same fake
// field the destination screen reads, which is exactly the handoff under test.
static uint32_t reason_set(void) {
    return fake_scene.scene_state;
}

// ---- the round-5 mode gate -------------------------------------------------------------------------

// A cut WIPE belongs on the wipe-specific screen.
static void test_cut_wipe_routes_to_wipe_stopped(void) {
    begin("a cut wipe routes to the Wipe stopped screen");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeWipe);
    app.iso15693_result.pass_truncated = true;
    send(NfcMagicCustomEventWorkerPartial);

    CHECK(routed_to() == NfcMagicSceneIso15693WriteFail);
    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonWipeStopped);
    end();
}

// THE MODE GATE. The same flag on a CLONE must not land on a screen whose every string says "wipe".
// Before round 5 pass_truncated was wipe-only, so this branch never had to ask.
static void test_cut_clone_does_not_route_to_wipe_stopped(void) {
    begin("a cut CLONE does not land on the wipe-specific screen");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    app.iso15693_result.pass_truncated = true;
    send(NfcMagicCustomEventWorkerPartial);

    CHECK(routed_to() == NfcMagicSceneIso15693WriteFail);
    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonPartial);
    CHECK(reason_set() != NfcMagicIso15693WriteFailReasonWipeStopped);
    end();
}

// A moved UID outranks a cut sweep: the card's identity changing under an operation that sends no UID
// command is the more surprising fact, whatever the block counts say.
static void test_uid_changed_outranks_truncation(void) {
    begin("a moved UID outranks a cut sweep");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeWipe);
    app.iso15693_result.uid_changed = true;
    app.iso15693_result.pass_truncated = true; // both true; uid_changed must win
    send(NfcMagicCustomEventWorkerPartial);

    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonWipeUidChanged);
    end();
}

// Neither qualifier: the ordinary partial screen.
static void test_plain_partial_routes_to_partial(void) {
    begin("a partial with neither qualifier routes to the ordinary partial screen");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    app.iso15693_result.failed_count = 3;
    send(NfcMagicCustomEventWorkerPartial);

    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonPartial);
    end();
}

// ---- protocol dispatch -----------------------------------------------------------------------------

// Gen2 and Classic share the gen2 partial screen; Classic is not a separate poller.
static void test_gen2_and_classic_share_their_partial_screen(void) {
    begin("Gen2 and Classic route to the Gen2 partial screen");
    setup(NfcMagicProtocolGen2, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventWorkerPartial);
    CHECK(routed_to() == NfcMagicSceneGen2WipePartial);

    setup(NfcMagicProtocolClassic, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventWorkerPartial);
    CHECK(routed_to() == NfcMagicSceneGen2WipePartial);
    end();
}

// Anything else falls through to the USCUID-UL screen.
static void test_other_protocols_route_to_uscuid_partial(void) {
    begin("other protocols route to the USCUID-UL partial screen");
    setup(NfcMagicProtocolUscuidUl, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventWorkerPartial);
    CHECK(routed_to() == NfcMagicSceneUscuidUlPartial);
    end();
}

// ---- success routing -------------------------------------------------------------------------------

// A wipe always goes to the result screen, because its sweep length is measured rather than assumed and
// the bare Success popup has nowhere to put the two figures.
static void test_wipe_success_goes_to_the_result_screen(void) {
    begin("a successful wipe goes to the result screen, not the Success popup");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeWipe);
    send(NfcMagicCustomEventWorkerSuccess);

    CHECK(routed_to() == NfcMagicSceneIso15693WriteFail);
    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonWipeComplete);
    end();
}

// A clone that left empty blocks past the card's capacity also has something to say.
static void test_over_capacity_clone_goes_to_the_result_screen(void) {
    begin("a clone with an empty over-capacity tail goes to the result screen");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    app.iso15693_result.over_capacity = 6;
    send(NfcMagicCustomEventWorkerSuccess);

    CHECK(routed_to() == NfcMagicSceneIso15693WriteFail);
    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonOverCapacity);
    end();
}

// A clean clone has nothing extra to report, so it gets the ordinary Success popup.
static void test_clean_clone_gets_the_success_popup(void) {
    begin("a clean clone gets the plain Success popup");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventWorkerSuccess);

    CHECK(routed_to() == NfcMagicSceneSuccess);
    end();
}

// Write-UID success refreshes the stored read result, so re-entering Write UID does not seed the editor
// from the pre-write UID and look as though nothing took.
static void test_write_uid_success_refreshes_the_stored_uid(void) {
    begin("a Write-UID success refreshes the stored UID");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeWriteUid);
    const uint8_t target[ISO15693_3_UID_SIZE] = {0xE0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    memcpy(app.iso15693_target_uid, target, sizeof(target));
    send(NfcMagicCustomEventWorkerSuccess);

    CHECK(memcmp(iso_data.uid, target, sizeof(target)) == 0);
    CHECK(routed_to() == NfcMagicSceneSuccess);
    end();
}

// ---- card lost -------------------------------------------------------------------------------------

// For ISO15693 a lost card is terminal, not a resumable search: the write may have half-happened.
static void test_iso15693_card_lost_is_terminal(void) {
    begin("ISO15693 treats a lost card as terminal");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventCardLost);

    CHECK(routed_to() == NfcMagicSceneIso15693WriteFail);
    CHECK(reason_set() == NfcMagicIso15693WriteFailReasonCardLost);
    end();
}

// The other protocols go back to searching, which is the pre-existing behaviour and must not change.
static void test_other_protocols_resume_the_search(void) {
    begin("other protocols resume the card search instead");
    setup(NfcMagicProtocolGen2, NfcMagicIso15693ModeClone);
    send(NfcMagicCustomEventCardLost);

    CHECK(routed_to() == UINT32_MAX); // no forward navigation at all
    CHECK(reason_set() == NfcMagicSceneWriteStateCardSearch);
    end();
}

// ---- Back swallowing -------------------------------------------------------------------------------

// Back is swallowed for ISO15693 once a card has been found, because the write cannot be aborted -- only
// discarded, and discarding it mid-sequence can leave the card in the "UID only" state.
static void test_back_is_swallowed_for_iso15693_once_a_card_is_found(void) {
    begin("Back is swallowed for ISO15693 after the card is found");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    fake_scene.scene_state = NfcMagicSceneWriteStateCardFound;

    SceneManagerEvent back = {.type = SceneManagerEventTypeBack, .event = 0};
    const bool consumed = nfc_magic_scene_write_on_event(&app, back);

    CHECK(consumed); // swallowed
    CHECK(fake_scene.nav_count == 0); // and it went nowhere
    end();
}

// ...but NOT during the card search, where nothing has been written and leaving is safe.
static void test_back_still_works_during_the_card_search(void) {
    begin("Back still leaves during the card search");
    setup(NfcMagicProtocolIso15693, NfcMagicIso15693ModeClone);
    fake_scene.scene_state = NfcMagicSceneWriteStateCardSearch;

    SceneManagerEvent back = {.type = SceneManagerEventTypeBack, .event = 0};
    const bool consumed = nfc_magic_scene_write_on_event(&app, back);

    CHECK(!consumed); // not swallowed: the scene manager handles it and the user leaves
    end();
}

// And not for the other four protocols, where swallowing it would be a trap: their pollers can stop
// advancing with the card gone, so Back is the only way off the popup. gen2/Classic, USCUID-direct and
// gen4 are the unsafe ones -- see the comment in the scene.
static void test_back_is_not_swallowed_for_other_protocols(void) {
    begin("Back is not swallowed for the other magic protocols");
    const NfcMagicProtocol others[] = {
        NfcMagicProtocolGen1,
        NfcMagicProtocolGen2,
        NfcMagicProtocolClassic,
        NfcMagicProtocolGen4,
        NfcMagicProtocolUscuidUl,
    };
    for(size_t i = 0; i < sizeof(others) / sizeof(others[0]); i++) {
        setup(others[i], NfcMagicIso15693ModeClone);
        fake_scene.scene_state = NfcMagicSceneWriteStateCardFound;
        SceneManagerEvent back = {.type = SceneManagerEventTypeBack, .event = 0};
        const bool consumed = nfc_magic_scene_write_on_event(&app, back);
        CHECK(!consumed);
        if(current_failed) printf("      (protocol index %zu swallowed Back)\n", i);
    }
    end();
}

int main(void) {
    printf("write scene routing\n");
    test_cut_wipe_routes_to_wipe_stopped();
    test_cut_clone_does_not_route_to_wipe_stopped();
    test_uid_changed_outranks_truncation();
    test_plain_partial_routes_to_partial();
    test_gen2_and_classic_share_their_partial_screen();
    test_other_protocols_route_to_uscuid_partial();
    test_wipe_success_goes_to_the_result_screen();
    test_over_capacity_clone_goes_to_the_result_screen();
    test_clean_clone_gets_the_success_popup();
    test_write_uid_success_refreshes_the_stored_uid();
    test_iso15693_card_lost_is_terminal();
    test_other_protocols_resume_the_search();
    test_back_is_swallowed_for_iso15693_once_a_card_is_found();
    test_back_still_works_during_the_card_search();
    test_back_is_not_swallowed_for_other_protocols();
    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
