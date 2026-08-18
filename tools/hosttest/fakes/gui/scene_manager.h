#pragma once

// Host-test fake for the scene manager. The two type definitions below are COPIED VERBATIM from
// applications/services/gui/scene_manager.h in Momentum 87.15 -- the scene's on_event reads
// event.type and event.event, so a wrong field name or a wrong enumerator order would make every
// navigation assertion meaningless. Re-check them if the firmware moves.
//
// The functions are recorded rather than implemented: see fake_scene.h.

#include <furi.h>

typedef enum {
    SceneManagerEventTypeCustom,
    SceneManagerEventTypeBack,
    SceneManagerEventTypeTick,
} SceneManagerEventType;

typedef struct {
    SceneManagerEventType type;
    uint32_t event;
} SceneManagerEvent;

typedef void (*AppSceneOnEnterCallback)(void* context);
typedef bool (*AppSceneOnEventCallback)(void* context, SceneManagerEvent event);
typedef void (*AppSceneOnExitCallback)(void* context);

typedef struct {
    const AppSceneOnEnterCallback* on_enter_handlers;
    const AppSceneOnEventCallback* on_event_handlers;
    const AppSceneOnExitCallback* on_exit_handlers;
    const uint32_t scene_num;
} SceneManagerHandlers;

typedef struct SceneManager SceneManager;

void scene_manager_set_scene_state(SceneManager* scene_manager, uint32_t scene_id, uint32_t state);
uint32_t scene_manager_get_scene_state(const SceneManager* scene_manager, uint32_t scene_id);
void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id);
bool scene_manager_previous_scene(SceneManager* scene_manager);
bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);
bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);
