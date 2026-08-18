#pragma once

// Host-test RECORDER for the widget module. Nothing is drawn; every element the scene adds is captured
// so a test can assert what the screen says and which buttons it offers.
//
// This is the point of the whole scene harness. Three of the four defects found in round 5 were in this
// layer -- a button labelled Exit that opened Details, a screen that never said why it stopped, and a
// note promising something the mechanism cannot do -- and none of them was reachable from the poller
// tests. A label and a route are both just data here, so "the label agrees with where on_event sends
// you" becomes an assertion instead of a thing someone notices on a 128x64 screen.

#include <furi.h>
#include <gui/view.h>
#include <input/input.h>

typedef struct Widget Widget;

// Enumerator lists here and below are copied from the firmware (widget_element.h, canvas.h) so the fake
// cannot drift from it. What the tests actually rely on is that a button type ROUND-TRIPS: the scene
// hands one to widget_add_button_element, the callback sends it as a custom event, and on_event compares
// against the same symbol -- so they only have to be distinct, not any particular number.
typedef enum {
    GuiButtonTypeLeft,
    GuiButtonTypeCenter,
    GuiButtonTypeRight,
} GuiButtonType;

typedef enum {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenter,
} Align;

typedef enum {
    FontPrimary,
    FontSecondary,
    FontKeyboard,
    FontBigNumbers,
    FontBatteryPercent,
    FontTotalNumber,
} Font;

typedef void (*ButtonCallback)(GuiButtonType result, InputType type, void* context);

// Icon is defined by the generated-icons stub, which the app header includes.
#include <nfc_magic_dev_icons.h>

void widget_reset(Widget* widget);

void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text);

void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon);
