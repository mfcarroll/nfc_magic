// Host-side tests for the ISO15693 result screens -- what they say, which buttons they offer, and where
// those buttons go.
//
// The shipped scenes are compiled VERBATIM: this file includes the .c files so the file-static predicates
// are reachable, and the GUI calls resolve to the recorders in fake_scene.c. No production code is
// modified or wrapped.
//
// Why this layer needed its own harness. Round 5 turned up four defects. The poller tests found the one
// that lived in the poller; the other three lived here and were found by a person reading the review or
// a 128x64 screen:
//
//   - a control labelled "Exit" that opened the Details scroll view, because on_enter branched on
//     is_retryable and on_event branched on has_details and the two disagreed
//   - a "Wipe stopped" screen that never said WHY it stopped, while offering Retry
//   - a Details note promising "Running the clone again writes them", which the wall-clock bound cannot
//     deliver
//
// The first of those is pure data here -- a label and a navigation target -- so it is exactly the kind of
// thing that should fail a test rather than survive to a review. The other two are string assertions,
// which pin them against silent drift.

#include "fake_scene.h"

#include "../../scenes/nfc_magic_scene_partial_details_common.c" // NOLINT -- deliberate, see above
#include "../../scenes/nfc_magic_scene_iso15693_partial_details.c" // NOLINT
#include "../../scenes/nfc_magic_scene_iso15693_write_fail.c" // NOLINT

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

#define CHECK_STR(actual, expected)                              \
    do {                                                         \
        const char* a_ = (actual);                               \
        if(a_ == NULL || strcmp(a_, (expected)) != 0) {          \
            printf(                                              \
                "  FAIL %s:%d  %s == \"%s\", expected \"%s\"\n", \
                __FILE__,                                        \
                __LINE__,                                        \
                #actual,                                         \
                a_ ? a_ : "(null)",                              \
                (expected));                                     \
            current_failed = true;                               \
        }                                                        \
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
        printf(
            "  --- what the scene drew ---\n%s  ---------------------------\n",
            fake_scene_all_text());
    } else {
        printf("  ok  %s\n", current_test);
    }
}

// ---- harness ---------------------------------------------------------------------------------------

static NfcMagicApp app;

// Set the app up as the write scene would have left it, then run the write-fail screen's on_enter.
static void render_write_fail(NfcMagicIso15693WriteFailReason reason, NfcMagicIso15693Mode mode) {
    memset(&app, 0, sizeof(app));
    app.protocol = NfcMagicProtocolIso15693;
    app.iso15693_mode = mode;
    fake_scene_reset((uint32_t)reason);
    nfc_magic_scene_iso15693_write_fail_on_enter(&app);
}

// As above but with a result already populated by the caller.
static void render_write_fail_with(
    NfcMagicIso15693WriteFailReason reason,
    NfcMagicIso15693Mode mode,
    const Iso15693PollerResult* result) {
    memset(&app, 0, sizeof(app));
    app.protocol = NfcMagicProtocolIso15693;
    app.iso15693_mode = mode;
    app.iso15693_result = *result;
    fake_scene_reset((uint32_t)reason);
    nfc_magic_scene_iso15693_write_fail_on_enter(&app);
}

static void render_details(NfcMagicIso15693WriteFailReason reason, NfcMagicIso15693Mode mode) {
    // Details is pushed on top of the write-fail scene and reads the SAME scene state for its reason,
    // so the fake keeps the state across the transition, exactly as the scene manager would.
    const uint32_t keep = fake_scene.scene_state;
    const Iso15693PollerResult result = app.iso15693_result;
    memset(&app, 0, sizeof(app));
    app.protocol = NfcMagicProtocolIso15693;
    app.iso15693_mode = mode;
    app.iso15693_result = result;
    fake_scene_reset(keep ? keep : (uint32_t)reason);
    nfc_magic_scene_iso15693_partial_details_on_enter(&app);
}

// Send a button through on_event the way the dispatcher does, and report where it navigated.
static FakeNav route_of(GuiButtonType button) {
    const uint16_t before = fake_scene.nav_count;
    SceneManagerEvent ev = {.type = SceneManagerEventTypeCustom, .event = (uint32_t)button};
    nfc_magic_scene_iso15693_write_fail_on_event(&app, ev);
    if(fake_scene.nav_count == before) {
        FakeNav none = {.kind = FakeNavNone, .scene_id = 0};
        return none;
    }
    return fake_scene.navs[fake_scene.nav_count - 1];
}

// ---- the cases -------------------------------------------------------------------------------------

// THE ROUND-5 BLOCKING BUG, as a test. on_enter chose the right-slot label from is_retryable while
// on_event routed it from has_details. WipeStopped is in both, so the label said Exit and the button
// opened Details. The invariant is that the label and the destination agree -- checked here for every
// reason code, so adding one to either predicate cannot reintroduce it silently.
static void test_right_button_label_matches_where_it_goes(void) {
    begin("every right-slot label agrees with where on_event sends it");
    const NfcMagicIso15693WriteFailReason reasons[] = {
        NfcMagicIso15693WriteFailReasonCardLost,
        NfcMagicIso15693WriteFailReasonNotMagic,
        NfcMagicIso15693WriteFailReasonPartial,
        NfcMagicIso15693WriteFailReasonOverCapacity,
        NfcMagicIso15693WriteFailReasonNothingWiped,
        NfcMagicIso15693WriteFailReasonNothingCloned,
        NfcMagicIso15693WriteFailReasonEmptySource,
        NfcMagicIso15693WriteFailReasonUidUnexpected,
        NfcMagicIso15693WriteFailReasonGen1Failed,
        NfcMagicIso15693WriteFailReasonUidUnverifiable,
        NfcMagicIso15693WriteFailReasonWipeUidChanged,
        NfcMagicIso15693WriteFailReasonWipeComplete,
        NfcMagicIso15693WriteFailReasonWipeStopped,
    };
    for(size_t i = 0; i < sizeof(reasons) / sizeof(reasons[0]); i++) {
        // A populated result, so reasons gated on failed_count still offer their button.
        Iso15693PollerResult r = {0};
        r.blocks_total = 64;
        r.blocks_advertised = 70;
        r.failed_count = 6;
        r.cut_block = 23;
        r.uid_verified = true;
        if(reasons[i] == NfcMagicIso15693WriteFailReasonWipeStopped) r.pass_truncated = true;

        render_write_fail_with(reasons[i], NfcMagicIso15693ModeWipe, &r);
        const char* label = fake_scene_button(GuiButtonTypeRight);
        if(label == NULL) continue; // no right button on this screen at all
        const FakeNav nav = route_of(GuiButtonTypeRight);
        if(strcmp(label, "Details") == 0) {
            CHECK(nav.kind == FakeNavNext);
            CHECK(nav.scene_id == NfcMagicSceneIso15693PartialDetails);
        } else {
            // Anything not labelled Details must LEAVE, never navigate deeper.
            CHECK(nav.kind == FakeNavSearchPrevious);
            CHECK(nav.scene_id == NfcMagicSceneIso15693);
        }
        if(current_failed) printf("      (reason index %zu, label \"%s\")\n", i, label);
    }
    end();
}

// Every reason code renders its OWN screen. This pins reason -> title, which is the mapping the switch
// exists to express, and it is the guarantee a `switch` on a uint32_t cannot get from -Wswitch: give a
// reason no arm of its own and it falls through to `default`, which renders a perfectly well-formed
// "Write failed" screen. That is a confidently wrong message, not a crash, so only an expected-title
// assertion catches it.
//
// The partial screen's title is mode-dependent, so it is checked in both modes.
static void test_every_reason_renders_its_own_screen(void) {
    begin("every reason code renders its own titled screen");
    struct {
        NfcMagicIso15693WriteFailReason reason;
        const char* title;
    } expected[] = {
        {NfcMagicIso15693WriteFailReasonWipeComplete, "Wipe complete"},
        {NfcMagicIso15693WriteFailReasonWipeStopped, "Wipe stopped"},
        {NfcMagicIso15693WriteFailReasonOverCapacity, "Clone finished"},
        {NfcMagicIso15693WriteFailReasonNothingWiped, "Wipe failed"},
        {NfcMagicIso15693WriteFailReasonNothingCloned, "Clone failed"},
        {NfcMagicIso15693WriteFailReasonWipeUidChanged, "UID changed"},
        {NfcMagicIso15693WriteFailReasonUidUnexpected, "Unexpected UID"},
        {NfcMagicIso15693WriteFailReasonGen1Failed, "gen1 failed"},
        {NfcMagicIso15693WriteFailReasonUidUnverifiable, "UID unchanged"},
        {NfcMagicIso15693WriteFailReasonEmptySource, "Nothing to clone"},
        // Both of these are the default arm, deliberately: it has to tell them apart in its body.
        {NfcMagicIso15693WriteFailReasonCardLost, "Write failed"},
        {NfcMagicIso15693WriteFailReasonNotMagic, "Write failed"},
    };
    for(size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        Iso15693PollerResult r = {0};
        r.blocks_total = 64;
        r.blocks_advertised = 70;
        r.failed_count = 6;
        r.over_capacity = 2;
        r.cut_block = 23;
        r.uid_verified = true;
        render_write_fail_with(expected[i].reason, NfcMagicIso15693ModeWipe, &r);

        const char* title = NULL;
        for(uint16_t e = 0; e < fake_scene.element_count; e++) {
            const FakeElement* el = &fake_scene.elements[e];
            if(el->kind == FakeElementString && el->font == FontPrimary) {
                title = el->text;
                break;
            }
        }
        CHECK_STR(title, expected[i].title);
        // A left button too, so there is always a labelled way off the screen.
        CHECK(fake_scene_button(GuiButtonTypeLeft) != NULL);
        if(current_failed) printf("      (reason index %zu)\n", i);
    }

    // Partial titles itself by mode.
    Iso15693PollerResult r = {0};
    r.blocks_total = 70;
    r.failed_count = 6;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeWipe, &r);
    CHECK(fake_scene_text_contains("Wipe partial"));
    render_write_fail_with(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone, &r);
    CHECK(fake_scene_text_contains("Clone partial"));
    end();
}

// The specific shape the fix chose: a cut sweep has something behind Details, so Details takes the right
// slot and Back is the exit. If someone puts "Exit" back here, the truncation note loses its only route.
static void test_wipe_stopped_offers_retry_and_details(void) {
    begin("a cut sweep offers Retry + Details, not Retry + Exit");
    Iso15693PollerResult r = {0};
    r.blocks_total = 23;
    r.blocks_advertised = 70;
    r.cut_block = 23;
    r.pass_truncated = true;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);

    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Retry");
    CHECK_STR(fake_scene_button(GuiButtonTypeRight), "Details");
    end();
}

// A card lifted mid-write has nothing behind Details, so the same rule yields Exit there. This is the
// direction the fix could have over-applied in.
static void test_card_lost_offers_retry_and_exit(void) {
    begin("card lost offers Retry + Exit, since it has no details");
    render_write_fail(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeClone);

    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Retry");
    CHECK_STR(fake_scene_button(GuiButtonTypeRight), "Exit");
    end();
}

// ...but a WIPE that lost the card is the opposite case, and the mode is what separates them. The sweep
// writes 56/57 at index 56/57, long before any plausible cut, and on an armed gen1 card those two blocks
// ARE the UID -- so the card can be gone and its identity gone with it. The wipe returns before
// Iso15693WriteStateVerifyWipe is ever entered, so uid_verified stays false and nothing ran to notice.
// Details is the only route to the sentence that says so.
static void test_wipe_card_lost_offers_details_for_the_uid_note(void) {
    begin("a wipe that lost the card offers Details, because the UID check never ran");
    Iso15693PollerResult r = {0};
    r.blocks_total = 40;
    r.uid_verified = false;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe, &r);

    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Retry");
    CHECK_STR(fake_scene_button(GuiButtonTypeRight), "Details");
    // The label has to agree with the destination -- this reason is now in BOTH predicates, which is
    // exactly the pairing that produced the round-5 "Exit opens Details" bug.
    CHECK(route_of(GuiButtonTypeRight).scene_id == NfcMagicSceneIso15693PartialDetails);
    end();
}

// Pins the DEFENSIVE uid_verified term, not a reachable outcome: the poller sets uid_verified only in
// Iso15693WriteStateVerifyWipe, whose one exit is success_or_partial, so it cannot pair that flag with
// CardLost. This fixes what the scene does if that ever changes -- it is not evidence that it happens.
static void test_wipe_card_lost_with_a_verified_uid_offers_exit(void) {
    begin("a wipe that lost the card after the UID check answered offers Exit");
    Iso15693PollerResult r = {0};
    r.blocks_total = 40;
    r.uid_verified = true;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe, &r);

    CHECK_STR(fake_scene_button(GuiButtonTypeRight), "Exit");
    end();
}

// The sentence itself, and its wording. A card-lost wipe never reached the field reset, so the note must
// not blame one -- "did not answer after the field reset" names a step that never happened here.
static void test_wipe_card_lost_details_says_the_check_never_finished(void) {
    begin("the card-lost Details note does not blame a field reset that never happened");
    Iso15693PollerResult r = {0};
    r.blocks_total = 40;
    r.uid_verified = false;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "UID not re-checked") != NULL);
        CHECK(strstr(scroll, "stopped answering") != NULL);
        CHECK(strstr(scroll, "field reset") == NULL);
    }
    end();
}

// And the list that must NOT appear. The poller's own sweep says a card lifted mid-wipe "can surface as
// a pile of blocks that wouldn't clear", and documents those counters as the caller's to discard on that
// exit -- so printing them would name the card's departure as a pile of refusals.
static void test_wipe_card_lost_details_lists_no_blocks(void) {
    begin("a card-lost wipe lists no blocks, only the UID note");
    Iso15693PollerResult r = {0};
    r.blocks_total = 40;
    r.uid_verified = false;
    // The shape the sweep really leaves behind: blocks that stopped answering as the card left.
    r.failed_bitmap[7 / 8] |= (uint8_t)(1u << (7 % 8));
    r.failed_bitmap[31 / 8] |= (uint8_t)(1u << (31 % 8));
    r.failed_count = 2;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeWipe);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "UID not re-checked") != NULL);
        // No block index, asserted as "no digit anywhere" rather than as a search for "7" -- a
        // single-character strstr passes on any text containing that character and so is one wording
        // change away from being vacuous. The UID note carries no digits, so this is exact.
        bool has_digit = false;
        for(const char* p = scroll; *p; p++) {
            if(*p >= '0' && *p <= '9') has_digit = true;
        }
        CHECK(!has_digit);
    }
    // Titled for what it actually shows, not for a list it does not have.
    CHECK(fake_scene_text_contains("Wipe notes"));
    CHECK(!fake_scene_text_contains("Blocks not cleared"));
    end();
}

// Found on hardware: the screen named where the sweep stopped and never said why, while offering a
// Retry button -- so the user was asked to retry against a cause the screen withheld.
static void test_wipe_stopped_says_it_timed_out(void) {
    begin("a cut sweep says it TIMED OUT, not just where it stopped");
    Iso15693PollerResult r = {0};
    r.blocks_total = 23;
    r.blocks_advertised = 70;
    r.cut_block = 23;
    r.pass_truncated = true;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);

    CHECK(fake_scene_text_contains("Timed out at block 23."));
    CHECK(fake_scene_text_contains("Wipe stopped"));
    end();
}

// The cut index, not blocks_total. On this trace they differ, which is the whole point: blocks_total is
// the highest block that ANSWERED and sits at or below the cut.
static void test_wipe_stopped_prints_the_cut_not_the_total(void) {
    begin("a cut sweep prints the cut index, never blocks_total");
    Iso15693PollerResult r = {0};
    r.blocks_total = 50; // proved present
    r.blocks_advertised = 64;
    r.cut_block = 55; // actually attempted this far
    r.pass_truncated = true;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);

    CHECK(fake_scene_text_contains("Timed out at block 55."));
    CHECK(!fake_scene_text_contains("block 50"));
    // And no "%u of %u" fraction, which read as "fell short of 64" when the cut had passed it.
    CHECK(!fake_scene_text_contains(" of 64"));
    end();
}

// A cut sweep is a partial outcome, so it must carry the error tone rather than the success chime.
static void test_cut_sweep_plays_the_error_tone(void) {
    begin("a cut sweep plays the error tone, a clean wipe the success tone");
    Iso15693PollerResult r = {0};
    r.pass_truncated = true;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    CHECK(fake_scene.played_error);
    CHECK(!fake_scene.played_success);

    render_write_fail(NfcMagicIso15693WriteFailReasonWipeComplete, NfcMagicIso15693ModeWipe);
    CHECK(fake_scene.played_success);
    CHECK(!fake_scene.played_error);
    end();
}

// A wipe whose identity check never reached an answer must say so. It had a line on "Wipe complete" and
// nowhere else, which is why it moved into Details.
static void test_unverified_uid_is_stated_on_wipe_complete(void) {
    begin("a wipe that could not re-check the UID says so");
    Iso15693PollerResult r = {0};
    r.blocks_total = 64;
    r.blocks_advertised = 70;
    r.uid_verified = false;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeComplete, NfcMagicIso15693ModeWipe, &r);

    CHECK(fake_scene_text_contains("UID not re-checked."));

    // ...and stays quiet when the check did run.
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeComplete, NfcMagicIso15693ModeWipe, &r);
    CHECK(!fake_scene_text_contains("UID not re-checked."));
    end();
}

// cut_block == blocks_advertised, the one boundary both sides of the review have had backwards. It is a
// COUNT against an INDEX: at equality the claimed blocks are 0..N-1 and the cut sits at index N, which is
// the first block PAST the claim. So the "of the N this card claims" wording must NOT be used there --
// it names an index that is not one of the N and reads as a completed fraction. Round 6 changed this to
// <=, round 7 asked for the revert; this pins it so a third round-trip is not possible.
static void test_cut_at_the_claim_reads_as_past_it(void) {
    begin(
        "a cut exactly at the advertised count reads as PAST the claim, not as a fraction of it");
    Iso15693PollerResult r = {0};
    r.blocks_total = 64;
    r.blocks_advertised = 64;
    r.cut_block = 64; // == the count, so index 64 is the first block past blocks 0..63
    r.pass_truncated = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "past the 64 this card claims") != NULL);
        CHECK(strstr(scroll, "of the 64 this card claims") == NULL);
    }

    // ...and one block lower still reads as inside the claim, so the boundary is the only thing moving.
    r.cut_block = 63;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe);
    scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "of the 64 this card claims") != NULL);
    }
    end();
}

// Found on hardware: the note promised that re-running writes the unsent blocks. The bound is a wall
// clock, so a consistently slow card is cut in the same place every time.
static void test_cut_clone_note_promises_nothing(void) {
    begin("the cut-clone note does not promise a retry will finish");
    Iso15693PollerResult r = {0};
    r.blocks_total = 256;
    r.failed_count = 246;
    r.cut_block = 10;
    r.pass_truncated = true;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone, &r);
    render_details(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "Running the clone again writes them") == NULL);
        CHECK(strstr(scroll, "the card did not refuse them") != NULL);
        CHECK(strstr(scroll, "may get further") != NULL);
    }
    end();
}

// The Details list must stop at the cut: below it are blocks the card refused, at and above it are
// blocks nothing was sent to, and listing them together names the second group as refusals.
static void test_cut_clone_lists_only_blocks_below_the_cut(void) {
    begin("a cut clone lists only the blocks below the cut");
    Iso15693PollerResult r = {0};
    r.blocks_total = 256;
    r.cut_block = 10;
    r.pass_truncated = true;
    // Block 3 genuinely refused; 10 and 200 are back-filled as unattempted.
    r.failed_bitmap[3 / 8] |= (uint8_t)(1u << (3 % 8));
    r.failed_bitmap[10 / 8] |= (uint8_t)(1u << (10 % 8));
    r.failed_bitmap[200 / 8] |= (uint8_t)(1u << (200 % 8));
    r.failed_count = 3;
    render_write_fail_with(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone, &r);
    render_details(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        CHECK(strstr(scroll, "3") != NULL); // the real refusal is listed
        CHECK(strstr(scroll, "200") == NULL); // the unattempted one is not
    }
    end();
}

// An UNCUT partial must keep listing the whole bitmap -- the new bound must not clip a run that was
// never cut. This is the half the gen2 card confirmed on hardware.
static void test_uncut_partial_lists_the_whole_bitmap(void) {
    begin("an uncut partial still lists every failed block");
    Iso15693PollerResult r = {0};
    r.blocks_total = 70;
    r.failed_count = 6;
    r.capacity_confirmed = true;
    for(uint16_t b = 64; b <= 69; b++)
        r.failed_bitmap[b / 8] |= (uint8_t)(1u << (b % 8));
    render_write_fail_with(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone, &r);
    render_details(NfcMagicIso15693WriteFailReasonPartial, NfcMagicIso15693ModeClone);

    const char* scroll = fake_scene_scroll_text();
    CHECK(scroll != NULL);
    if(scroll) {
        for(int b = 64; b <= 69; b++) {
            char want[8];
            snprintf(want, sizeof(want), "%d", b);
            CHECK(strstr(scroll, want) != NULL);
        }
    }
    end();
}

// The wipe's Details note picks its sentence from which side of the advertised count the cut fell on,
// because which side it is changes what is true.
static void test_wipe_note_switches_on_which_side_of_the_claim(void) {
    begin("the wipe note reads differently above and below the card's claim");
    Iso15693PollerResult r = {0};
    r.blocks_total = 50;
    r.blocks_advertised = 64;
    r.cut_block = 55; // inside the claim
    r.pass_truncated = true;
    r.failed_count = 1;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe);
    const char* inside = fake_scene_scroll_text();
    CHECK(inside && strstr(inside, "of the 64 this card claims") != NULL);

    r.cut_block = 200; // past the claim -- the "Stopped at 200 of 64" case
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    render_details(NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe);
    const char* past = fake_scene_scroll_text();
    CHECK(past && strstr(past, "past the 64 this card claims") != NULL);
    CHECK(past && strstr(past, "Every claimed block was attempted") != NULL);
    end();
}

// The left button is the primary action and its meaning differs: Retry re-runs the write, Finish and
// Back leave. Getting these crossed would either strand the user or silently repeat a destructive write.
static void test_left_button_routes_by_retryability(void) {
    begin("the left button re-runs only where it says Retry");
    Iso15693PollerResult r = {0};
    r.pass_truncated = true;
    r.uid_verified = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Retry");
    CHECK(route_of(GuiButtonTypeLeft).kind == FakeNavPrevious); // back to the write scene

    render_write_fail(NfcMagicIso15693WriteFailReasonWipeComplete, NfcMagicIso15693ModeWipe);
    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Finish");
    const FakeNav nav = route_of(GuiButtonTypeLeft);
    CHECK(nav.kind == FakeNavSearchPrevious);
    CHECK(nav.scene_id == NfcMagicSceneIso15693);
    end();
}

// Back must always leave, from any of these screens -- it is the exit on the screen that spends both
// button slots.
static void test_back_always_leaves(void) {
    begin("Back leaves from the write-fail screen whatever the reason");
    Iso15693PollerResult r = {0};
    r.pass_truncated = true;
    render_write_fail_with(
        NfcMagicIso15693WriteFailReasonWipeStopped, NfcMagicIso15693ModeWipe, &r);
    SceneManagerEvent back = {.type = SceneManagerEventTypeBack, .event = 0};
    nfc_magic_scene_iso15693_write_fail_on_event(&app, back);
    CHECK(fake_scene.nav_count > 0);
    if(fake_scene.nav_count) {
        CHECK(fake_scene.navs[fake_scene.nav_count - 1].kind == FakeNavSearchPrevious);
        CHECK(fake_scene.navs[fake_scene.nav_count - 1].scene_id == NfcMagicSceneIso15693);
    }
    end();
}

// A pressed button reaches on_event as its own type, via the widget callback. If this round-trip broke,
// every routing test above would be vacuous.
static void test_pressing_a_button_sends_its_own_type(void) {
    begin("pressing a button sends that button's type as the custom event");
    render_write_fail(NfcMagicIso15693WriteFailReasonCardLost, NfcMagicIso15693ModeClone);

    CHECK(fake_scene_press(GuiButtonTypeLeft, &app));
    CHECK(fake_scene.custom_event_sent);
    CHECK(fake_scene.custom_event == GuiButtonTypeLeft);

    CHECK(fake_scene_press(GuiButtonTypeRight, &app));
    CHECK(fake_scene.custom_event == GuiButtonTypeRight);
    end();
}

// The saturation the three "how many succeeded" lines share. blocks_total and failed_count come from
// different accountings and have disagreed before, so the promise here is about what reaches the user
// if that recurs: 0, not a number near 65535.
static void test_blocks_ok_saturates_instead_of_wrapping(void) {
    begin("the succeeded-blocks figure saturates at zero rather than wrapping");
    CHECK(nfc_magic_iso15693_blocks_ok(64, 4) == 60);
    CHECK(nfc_magic_iso15693_blocks_ok(20, 20) == 0);
    CHECK(nfc_magic_iso15693_blocks_ok(0, 0) == 0);
    // The case it exists for: more failures than the measured total.
    CHECK(nfc_magic_iso15693_blocks_ok(20, 44) == 0);
    CHECK(nfc_magic_iso15693_blocks_ok(0, 1) == 0);
    end();
}

int main(void) {
    printf("iso15693 result screens\n");
    test_right_button_label_matches_where_it_goes();
    test_every_reason_renders_its_own_screen();
    test_wipe_stopped_offers_retry_and_details();
    test_card_lost_offers_retry_and_exit();
    test_wipe_card_lost_offers_details_for_the_uid_note();
    test_wipe_card_lost_with_a_verified_uid_offers_exit();
    test_wipe_card_lost_details_says_the_check_never_finished();
    test_wipe_card_lost_details_lists_no_blocks();
    test_wipe_stopped_says_it_timed_out();
    test_wipe_stopped_prints_the_cut_not_the_total();
    test_cut_sweep_plays_the_error_tone();
    test_unverified_uid_is_stated_on_wipe_complete();
    test_cut_at_the_claim_reads_as_past_it();
    test_cut_clone_note_promises_nothing();
    test_cut_clone_lists_only_blocks_below_the_cut();
    test_uncut_partial_lists_the_whole_bitmap();
    test_wipe_note_switches_on_which_side_of_the_claim();
    test_left_button_routes_by_retryability();
    test_back_always_leaves();
    test_pressing_a_button_sends_its_own_type();
    test_blocks_ok_saturates_instead_of_wrapping();
    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
