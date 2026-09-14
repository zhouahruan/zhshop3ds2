/**
 * Scene stack manager.
 *
 * A scene instance is shared (reused) across pushes; scene managers
 * ensure on_enter/on_exit are paired correctly. State inside scene->state
 * is owned by the scene and may be reset in on_enter.
 */
#ifndef CORE_SCENE_MANAGER_H
#define CORE_SCENE_MANAGER_H

#include "scene.h"

void scene_manager_init(void);
void scene_manager_shutdown(void);

/* Push a new scene on the stack. on_pause is called on the previous top,
 * on_enter(args) on the new top. */
void scene_manager_push(Scene* s, void* args);

/* Pop the top scene. on_exit on the popped, on_resume on the new top. */
void scene_manager_pop(void);

/* Replace the top scene (pop+push). Useful for tab switching. */
void scene_manager_switch(Scene* s, void* args);

/* Reset stack to a single scene. */
void scene_manager_reset(Scene* s, void* args);

Scene* scene_manager_current(void);

/* Drives the top scene. */
void scene_manager_update(void);

/* Draws top scene to both screens. Transition can split draws. */
void scene_manager_draw(ScreenId screen);

#endif /* CORE_SCENE_MANAGER_H */
