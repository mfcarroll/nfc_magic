#pragma once
// Host-test recorder declarations; implemented in fake_scene.c.
#include <furi.h>
#include <gui/view.h>

typedef struct ViewDispatcher ViewDispatcher;

void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);
