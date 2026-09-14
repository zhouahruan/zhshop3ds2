/**
 * Scene interface. A scene owns the layout of both screens for the
 * duration it is on the stack. The scene manager keeps a stack so that
 * detail/detail-of-detail style navigation is natural (push/pop).
 */
#ifndef CORE_SCENE_H
#define CORE_SCENE_H

#include "render.h"

struct Scene;
typedef struct Scene Scene;

typedef struct SceneVTable {
    const char* name;
    void (*on_enter)(Scene* self, void* args);
    void (*on_exit)(Scene* self);
    void (*on_pause)(Scene* self);   /* called when another scene pushed on top */
    void (*on_resume)(Scene* self);  /* called when a popped scene returns focus */
    void (*update)(Scene* self);
    void (*draw)(Scene* self, ScreenId screen);
} SceneVTable;

struct Scene {
    const SceneVTable* vtable;
    void* state;          /* scene-private heap data */
};

/* Convenience macro to declare a scene accessor. */
#define DECLARE_SCENE(scene_name) \
    Scene* scene_##scene_name##_get(void)

#endif /* CORE_SCENE_H */
