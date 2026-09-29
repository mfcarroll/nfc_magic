// Host-side tests for the gen1 opt-in screen -- the one screen where a wrong button destroys a card:
// "Try gen1" writes blocks 56/57/62/63 with ordinary writes, on a tag that may not be magic at all.
// Pinned here: each label does what it says (only the right one grants the gen1 run, and both ways back
// leave the card alone), and each flow's body states what it will write.
//
// Same technique as test_write_fail_scene.c: the scene is compiled verbatim against the fakes, and a
// button is pressed through the callback it registered, the way the GUI presses it.

#include "fake_scene.h"

#include "../../scenes/nfc_magic_scene_iso15693_gen1_optin.c" // NOLINT -- deliberate, see above

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

#define CHECK_STR(actual, expected)                                                          \
    do {                                                                                     \
        const char* a_ = (actual);                                                           \
        if(a_ == NULL || strcmp(a_, (expected)) != 0) {                                      \
            printf(                                                                          \
                "  FAIL %s:%d  %s == \"%s\", expected \"%s\"\n",                             \
                __FILE__,                                                                    \
                __LINE__,                                                                    \
                #actual,                                                                     \
                a_ ? a_ : "(null)",                                                          \
                (expected));                                                                 \
            current_failed = true;                                                           \
        }                                                                                    \
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

// Whether the file holds data at 56/57/62/63 is the poller's to decide, and is tested there. Here it is
// a switch the test sets.
static bool file_uses_gen1_blocks;
bool iso15693_poller_source_uses_gen1_blocks(const Iso15693_3Data* data) {
    (void)data;
    return file_uses_gen1_blocks;
}

static void render(NfcMagicIso15693Gen1OptinSource from, bool uses_gen1_blocks) {
    memset(&app, 0, sizeof(app));
    file_uses_gen1_blocks = uses_gen1_blocks;
    fake_scene_reset((uint32_t)from);
    nfc_magic_scene_iso15693_gen1_optin_on_enter(&app);
}

static FakeNav last_nav_since(uint16_t before) {
    if(fake_scene.nav_count == before) {
        FakeNav none = {.kind = FakeNavNone, .scene_id = 0};
        return none;
    }
    return fake_scene.navs[fake_scene.nav_count - 1];
}

// Press a button through its callback, then feed the event that callback sent back through on_event.
static FakeNav press(GuiButtonType which) {
    const uint16_t before = fake_scene.nav_count;
    CHECK(fake_scene_press(which, &app));
    CHECK(fake_scene.custom_event_sent);
    SceneManagerEvent ev = {.type = SceneManagerEventTypeCustom, .event = fake_scene.custom_event};
    nfc_magic_scene_iso15693_gen1_optin_on_event(&app, ev);
    return last_nav_since(before);
}

// ---- the buttons -----------------------------------------------------------------------------------

static void test_try_gen1_grants_the_gen1_run(void) {
    begin("\"Try gen1\" is the right button, and it grants the gen1 run and re-runs the write");
    render(NfcMagicIso15693Gen1OptinFromClone, false);
    CHECK_STR(fake_scene_button(GuiButtonTypeRight), "Try gen1");
    const FakeNav nav = press(GuiButtonTypeRight);
    CHECK(app.iso15693_force_gen1);
    CHECK(nav.kind == FakeNavNext);
    CHECK(nav.scene_id == NfcMagicSceneWrite);
    end();
}

static void test_the_back_button_leaves_the_card_alone(void) {
    begin("\"Back\" declines: no gen1 grant, and back to the ISO15693 menu");
    render(NfcMagicIso15693Gen1OptinFromClone, false);
    CHECK_STR(fake_scene_button(GuiButtonTypeLeft), "Back");
    const FakeNav nav = press(GuiButtonTypeLeft);
    CHECK(!app.iso15693_force_gen1);
    CHECK(nav.kind == FakeNavSearchPrevious);
    CHECK(nav.scene_id == NfcMagicSceneIso15693);
    end();
}

static void test_the_back_key_leaves_the_card_alone(void) {
    begin("the Back key declines the same way");
    render(NfcMagicIso15693Gen1OptinFromWriteUid, false);
    const uint16_t before = fake_scene.nav_count;
    SceneManagerEvent ev = {.type = SceneManagerEventTypeBack, .event = 0};
    CHECK(nfc_magic_scene_iso15693_gen1_optin_on_event(&app, ev));
    const FakeNav nav = last_nav_since(before);
    CHECK(!app.iso15693_force_gen1);
    CHECK(nav.kind == FakeNavSearchPrevious);
    CHECK(nav.scene_id == NfcMagicSceneIso15693);
    end();
}

// ---- what each flow says it will write ------------------------------------------------------------

static void test_each_flow_states_what_it_writes(void) {
    begin("each flow's body names the four blocks and what else gen1 would write");
    render(NfcMagicIso15693Gen1OptinFromWriteUid, false);
    CHECK(fake_scene_text_contains("56/57/62/63"));
    CHECK(fake_scene_text_contains("Nothing else is written."));
    CHECK(!fake_scene_text_contains("the rest of the data"));

    render(NfcMagicIso15693Gen1OptinFromClone, false);
    CHECK(fake_scene_text_contains("56/57/62/63"));
    CHECK(fake_scene_text_contains("then the rest of the data only if that UID takes"));
    CHECK(fake_scene_text_contains("loses at most those 4 blocks"));
    CHECK(!fake_scene_text_contains("can't be cloned by gen1"));
    end();
}

// The warning that a file's own data at 56/57/62/63 cannot survive gen1 belongs on the clone's consent,
// the only point a clone asks -- and never on a Write UID, which has no file.
static void test_a_file_using_the_four_blocks_is_warned_about_on_a_clone_only(void) {
    begin("a file with data at 56/57/62/63 is warned about on a clone, never on a Write UID");
    render(NfcMagicIso15693Gen1OptinFromClone, true);
    CHECK(fake_scene_text_contains("can't be cloned by gen1"));

    render(NfcMagicIso15693Gen1OptinFromWriteUid, true);
    CHECK(!fake_scene_text_contains("can't be cloned by gen1"));
    end();
}

int main(void) {
    test_try_gen1_grants_the_gen1_run();
    test_the_back_button_leaves_the_card_alone();
    test_the_back_key_leaves_the_card_alone();
    test_each_flow_states_what_it_writes();
    test_a_file_using_the_four_blocks_is_warned_about_on_a_clone_only();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
