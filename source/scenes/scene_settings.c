#include "scene_settings.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../data/app_cache.h"
#include "../net/api_config.h"
#include "../net/http.h"
#include "../ui/button.h"
#include "../ui/modal.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Button back_btn;
    Button clear_cache_btn;
    Button test_net_btn;
    Modal  modal;
    int cursor;   /* 0=back, 1=clear cache, 2=test network */
} SceneSettingsState;

static void settings_on_back(void* ud) {
    (void)ud;
    scene_manager_pop();
}

static void settings_on_clear_cache(void* ud) {
    SceneSettingsState* st = (SceneSettingsState*)ud;
    cache_clear();
    modal_show_info(&st->modal, "Cache", "Cleared.", NULL, NULL);
}

static void settings_on_test_net(void* ud) {
    SceneSettingsState* st = (SceneSettingsState*)ud;
    NetResult r;
    memset(&r, 0, sizeof(r));
    NetStatus s = http_get(API_BASE_URL, &r);
    if (s == NET_OK && r.data)
        modal_show_info(&st->modal, "Network", "OK: reachable.", NULL, NULL);
    else
        modal_show_info(&st->modal, "Network", "Failed: unreachable.", NULL, NULL);
    http_result_free(&r);
}

static void settings_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneSettingsState));
    SceneSettingsState* st = (SceneSettingsState*)self->state;
    store_set_route("settings");
    button_init(&st->back_btn, 4, 4, 100, 30, "Back", C_MUTED, C_SURFACE,
                settings_on_back, NULL);
    button_init(&st->clear_cache_btn, 4, 40, 140, 30, "Clear Cache", C_BLUE, C_SURFACE,
                settings_on_clear_cache, st);
    button_init(&st->test_net_btn, 4, 76, 140, 30, "Test Network", C_BLUE, C_SURFACE,
                settings_on_test_net, st);
    modal_init(&st->modal);
    st->cursor = 0;
}

static void settings_update(Scene* self) {
    SceneSettingsState* st = (SceneSettingsState*)self->state;
    if (!st) return;

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (modal_handle_touch(&st->modal, t)) return;
        if (widget_touch(&st->back_btn.base, t)) return;
        if (widget_touch(&st->clear_cache_btn.base, t)) return;
        widget_touch(&st->test_net_btn.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    uint8_t nav = input_nav_pressed();
    if (nav & NAV_UP)   { st->cursor--; if (st->cursor < 0) st->cursor = 0; }
    if (nav & NAV_DOWN) { st->cursor++; if (st->cursor > 2) st->cursor = 2; }
    if (input_pressed(KEY_A_3DS)) {
        if      (st->cursor == 0) settings_on_back(NULL);
        else if (st->cursor == 1) settings_on_clear_cache(st);
        else                       settings_on_test_net(st);
    }
    if (input_pressed(KEY_B_3DS)) {
        settings_on_back(NULL);
    }
}

static void settings_draw(Scene* self, ScreenId screen) {
    SceneSettingsState* st = (SceneSettingsState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        render_draw_text(8, 4, 0.9f, 0.9f, C_TEXT, "Settings");
        render_draw_rect(8, 28, TOP_W - 16, 1, C_MUTED);
        float y = 40;
#if USE_MOCK
        render_draw_text(8, y, 0.6f, 0.6f, C_MUTED, "Data source: Mock (built-in)");
        y += 16;
#else
        render_draw_text(8, y, 0.6f, 0.6f, C_MUTED, "Data source: Live HTTP");
        y += 16;
        render_draw_text(8, y, 0.6f, 0.6f, C_MUTED, "API: %s", API_BASE_URL);
        y += 16;
        render_draw_text(8, y, 0.6f, 0.6f, C_MUTED, "Timeout: %d ms", API_TIMEOUT_MS);
        y += 16;
#endif
        render_draw_text(8, y, 0.6f, 0.6f, C_MUTED, "Route: %s", g_store.current_route);
        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "Up/Down: navigate  |  A: select  |  B: back");
        modal_draw_top(&st->modal);
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->back_btn.base);
        widget_draw(&st->clear_cache_btn.base);
        widget_draw(&st->test_net_btn.base);

        /* Gamepad cursor highlight. */
        float cy = (st->cursor == 0) ? 3.0f :
                   (st->cursor == 1) ? 39.0f : 75.0f;
        float ch = (st->cursor == 0) ? 32.0f : 32.0f;
        render_draw_rect_outline(3, cy, 146, ch, 2.0f, C_BLUE);

        modal_draw_bottom(&st->modal);
    }
}

static const SceneVTable s_settings_vtable = {
    .name = "settings",
    .on_enter  = settings_on_enter,
    .on_exit   = NULL,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = settings_update,
    .draw      = settings_draw,
};

static Scene s_settings_scene = {
    .vtable = &s_settings_vtable,
    .state  = NULL,
};

Scene* scene_settings_get(void) {
    return &s_settings_scene;
}
