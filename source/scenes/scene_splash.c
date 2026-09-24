#include "scene_splash.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t enter_ms;
    uint8_t  done;
} SplashState;

static void splash_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SplashState));
    SplashState* st = (SplashState*)self->state;
    st->enter_ms = osGetTime();
    st->done = 0;
}

static void splash_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static void splash_update(Scene* self) {
    SplashState* st = (SplashState*)self->state;
    if (!st) return;
    uint64_t elapsed = osGetTime() - st->enter_ms;
    if (elapsed > 1500 || input_pressed(KEY_A_3DS)) {
        if (!st->done) {
            st->done = 1;
            /* On first launch go to guide, otherwise go home. */
            extern Scene* scene_guide_get(void);
            extern Scene* scene_home_get(void);
            /* Always show guide for now — it has a skip option. */
            scene_manager_switch(scene_guide_get(), NULL);
        }
    }
}

static void splash_draw(Scene* self, ScreenId screen) {
    (void)self;
    if (screen == SCREEN_TOP) {
        render_clear(C_BG);
        render_draw_text(60, 100, 1.5f, 1.5f, C_YELLOW,
                         "3DS App Store");
        render_draw_text(120, 130, 0.8f, 0.8f, C_SURFACE,
                         "loading...");
    } else {
        render_clear(C_BG);
        render_draw_text(80, 110, 0.9f, 0.9f, C_MUTED,
                         "press A to skip");
    }
}

static const SceneVTable s_splash_vtable = {
    .name = "splash",
    .on_enter  = splash_on_enter,
    .on_exit   = splash_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = splash_update,
    .draw      = splash_draw,
};

static Scene s_splash_scene = {
    .vtable = &s_splash_vtable,
    .state  = NULL,
};

Scene* scene_splash_get(void) {
    return &s_splash_scene;
}
