// Recorders behind the scene fakes: what a scene drew, which buttons it offered, where it navigated,
// and which tone it played. Nothing is rendered -- the point is that every one of those is data a test
// can assert on.
//
// Why this exists: the poller tests found the one round-5 defect that lived in the poller, and missed
// all three that lived in the scenes. Two of those three were only ever going to be caught by a person
// reading a 128x64 screen, and one of them (a control labelled Exit that opened Details) was a pure
// disagreement between two functions in the same file -- exactly the thing a test should catch first.

#pragma once

#include <furi.h>
#include <gui/modules/widget.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#define FAKE_SCENE_MAX_ELEMENTS 24
#define FAKE_SCENE_MAX_NAV 8

typedef enum {
    FakeElementString, // widget_add_string_element
    FakeElementMultiline, // widget_add_string_multiline_element
    FakeElementScroll, // widget_add_text_scroll_element
    FakeElementButton, // widget_add_button_element
    FakeElementIcon, // widget_add_icon_element
} FakeElementKind;

typedef struct {
    FakeElementKind kind;
    // The scroll element's real counterpart copies into a FuriString and has no ceiling; this one
    // is a fixed buffer only so the fake can stay simple. Sized for the notes page, whose
    // paragraphs stack and which overran 512 once they started naming both sides of a mismatch.
    char text[2048]; // the rendered string, or the button label
    uint8_t x, y;
    Align horizontal, vertical;
    Font font;
    GuiButtonType button; // FakeElementButton only
    ButtonCallback callback; // FakeElementButton only -- so a test can press it
} FakeElement;

typedef enum {
    FakeNavNone,
    FakeNavNext, // scene_manager_next_scene
    FakeNavPrevious, // scene_manager_previous_scene
    FakeNavSearchPrevious, // scene_manager_search_and_switch_to_previous_scene
    FakeNavSearchAnother, // scene_manager_search_and_switch_to_another_scene
} FakeNavKind;

typedef struct {
    FakeNavKind kind;
    uint32_t scene_id; // meaningless for FakeNavPrevious, which takes no id
} FakeNav;

typedef struct {
    FakeElement elements[FAKE_SCENE_MAX_ELEMENTS];
    uint16_t element_count;

    FakeNav navs[FAKE_SCENE_MAX_NAV];
    uint16_t nav_count;

    bool played_success; // notification_message(&sequence_success)
    bool played_error; // notification_message(&sequence_error)

    uint32_t scene_state; // what scene_manager_get_scene_state returns
    uint32_t custom_event; // last view_dispatcher_send_custom_event payload
    bool custom_event_sent;
    bool switched_view;
    bool widget_was_reset;
} FakeScene;

extern FakeScene fake_scene;

// Clear everything and set the reason code the scene will read back out of the scene manager.
void fake_scene_reset(uint32_t scene_state);

// ---- queries, so the tests read as claims about the screen ---------------------------------------

// The label on a given button slot, or NULL if that slot has no button.
const char* fake_scene_button(GuiButtonType which);

// Every string the scene drew, joined with '\n' -- for asserting a phrase appears at all.
const char* fake_scene_all_text(void);

// True if any drawn string contains `needle`.
bool fake_scene_text_contains(const char* needle);

// The single scroll-view body, or NULL. This is the Details screen's text.
const char* fake_scene_scroll_text(void);

// Press a button: invokes the recorded callback the way the GUI would (InputTypeShort), which is what
// puts the button type into custom_event. Returns false if that slot has no button.
bool fake_scene_press(GuiButtonType which, void* context);
