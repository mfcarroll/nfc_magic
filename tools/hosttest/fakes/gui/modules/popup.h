#pragma once
// Host-test stub. The write scene sets popup text as it advances; the routing under test does not depend
// on any of it, so these record nothing. If a test ever needs the popup's text, promote them to
// recorders in fake_scene.c the way the widget calls are.
#include <furi.h>
#include <gui/modules/widget.h>
#include <gui/view.h>

typedef struct Popup Popup;

void popup_reset(Popup* popup);
void popup_set_header(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);
void popup_set_text(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);
void popup_set_icon(Popup* popup, uint8_t x, uint8_t y, const Icon* icon);
