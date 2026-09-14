#include "scene_home.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../net/api.h"
#include "../ui/grid.h"
#include "../ui/carousel.h"
#include "../ui/tabs.h"
#include "../ui/button.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Carousel carousel;
    Grid     grid;
    Tabs     tabs;
    Button   settings_btn;
    const char* cat_labels[MAX_CATEGORIES];
    int      cat_count;
    int      grid_cursor;       /* gamepad cursor index in grid */
    int      tab_cursor;        /* gamepad cursor index in tabs */
    uint8_t  focus_mode;        /* 0=grid, 1=tabs */
    uint8_t  loaded;
} SceneHomeState;

static const char** home_grid_label(int idx, void* ud) {
    (void)ud;
    static const char* slot = NULL;
    if (idx < 0 || idx >= g_store.recommend.count) { slot = NULL; return &slot; }
    slot = g_store.recommend.items[idx].name;
    return &slot;
}

static void home_open_detail(int idx) {
    if (idx < 0 || idx >= g_store.recommend.count) return;
    App* app = &g_store.recommend.items[idx];
    if (g_store.current_detail) { free(g_store.current_detail); g_store.current_detail = NULL; }
    g_store.current_detail = calloc(1, sizeof(AppDetail));
    if (!g_store.current_detail) return;
    api.get_app_detail(app->id, g_store.current_detail);
    extern Scene* scene_detail_get(void);
    scene_manager_switch(scene_detail_get(), NULL);
}

static void home_on_select_grid(int idx, void* ud) {
    (void)ud;
    home_open_detail(idx);
}

static void home_on_change_tab(int new_index, void* ud) {
    (void)ud;
    if (new_index < 0 || new_index >= g_store.categories.count) return;
    utils_strlcpy(g_store.list_category, g_store.categories.items[new_index].id,
                  sizeof(g_store.list_category));
    g_store.list_page = 1;
    g_store.loading = 1;
    extern Scene* scene_applist_get(void);
    scene_manager_switch(scene_applist_get(), NULL);
}

static void home_on_settings(void* ud) {
    (void)ud;
    extern Scene* scene_settings_get(void);
    scene_manager_push(scene_settings_get(), NULL);
}

static void home_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneHomeState));
    SceneHomeState* st = (SceneHomeState*)self->state;
    store_set_route("home");
    if (!st->loaded) {
        api.get_recommend(&g_store.recommend);
        api.get_categories(&g_store.categories);
        st->loaded = 1;
    }
    st->cat_count = g_store.categories.count;
    if (st->cat_count > MAX_CATEGORIES) st->cat_count = MAX_CATEGORIES;
    for (int i = 0; i < st->cat_count; ++i)
        st->cat_labels[i] = g_store.categories.items[i].name;

    carousel_init(&st->carousel, 0, 0, TOP_W, 92);
    grid_init(&st->grid, 4, 44, BOTTOM_W - 8, 152, 4, 4, 4,
             home_on_select_grid, NULL);
    grid_set_data(&st->grid, g_store.recommend.count, home_grid_label, NULL);
    tabs_init(&st->tabs, 4, 0, BOTTOM_W - 8, 40, st->cat_labels, st->cat_count,
              0, C_BLUE, C_SURFACE, C_TEXT, home_on_change_tab, NULL);
    button_init(&st->settings_btn, BOTTOM_W - 60, 4, 56, 32, "Set",
                C_SURFACE, C_TEXT, home_on_settings, NULL);
    st->grid_cursor = 0;
    st->tab_cursor = 0;
    st->focus_mode = 0;
}

static void home_update(Scene* self) {
    SceneHomeState* st = (SceneHomeState*)self->state;
    if (!st) return;
    carousel_update(&st->carousel, osGetTime());

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->settings_btn.base, t)) return;
        if (widget_touch(&st->tabs.base, t)) return;
        widget_touch(&st->grid.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    /* L/R to switch between grid and tabs. */
    if (input_pressed(KEY_L_3DS)) { st->focus_mode = 1; return; }
    if (input_pressed(KEY_R_3DS)) { st->focus_mode = 0; return; }

    uint8_t nav = input_nav_pressed();
    if (st->focus_mode == 0) {
        /* Grid navigation. */
        int cols = 4;
        int total = g_store.recommend.count;
        if (total <= 0) return;
        if (nav & NAV_LEFT)  { st->grid_cursor--; if (st->grid_cursor < 0) st->grid_cursor = total - 1; }
        if (nav & NAV_RIGHT) { st->grid_cursor = (st->grid_cursor + 1) % total; }
        if (nav & NAV_UP)    { st->grid_cursor -= cols; if (st->grid_cursor < 0) st->grid_cursor = 0; }
        if (nav & NAV_DOWN)  { st->grid_cursor += cols; if (st->grid_cursor >= total) st->grid_cursor = total - 1; }
        if (input_pressed(KEY_A_3DS)) {
            home_open_detail(st->grid_cursor);
        }
    } else {
        /* Tab navigation. */
        if (nav & NAV_LEFT)  { st->tab_cursor--; if (st->tab_cursor < 0) st->tab_cursor = st->cat_count - 1; }
        if (nav & NAV_RIGHT) { st->tab_cursor = (st->tab_cursor + 1) % st->cat_count; }
        if (input_pressed(KEY_A_3DS)) {
            home_on_change_tab(st->tab_cursor, NULL);
        }
    }
}

static void home_draw(Scene* self, ScreenId screen) {
    SceneHomeState* st = (SceneHomeState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "3DS App Store");
        render_draw_rounded_rect(8, 18, TOP_W - 16, 70, 6.0f, C_BLUE);
        render_draw_text(16, 44, 0.9f, 0.9f, C_SURFACE, "Featured Apps");
        widget_draw(&st->carousel.base);

        /* Draw gamepad hint. */
        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "D-pad/C-pad: navigate  |  A: select  |  B: back  |  L/R: switch panel");
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->settings_btn.base);
        widget_draw(&st->tabs.base);
        widget_draw(&st->grid.base);

        /* Draw gamepad cursor highlight. */
        if (st->focus_mode == 0 && g_store.recommend.count > 0) {
            float cw = (BOTTOM_W - 8 - 4 * 4) / 4.0f;
            float ch = (152.0f - 3 * 4) / 4.0f;
            int col = st->grid_cursor % 4;
            int row = st->grid_cursor / 4;
            float cx = 4 + col * (cw + 4);
            float cy = 44 + row * (ch + 4);
            render_draw_rect_outline(cx - 1, cy - 1, cw + 2, ch + 2, 2.0f, C_BLUE);
        } else if (st->focus_mode == 1 && st->cat_count > 0) {
            /* Highlight current tab. */
            float tab_w = (BOTTOM_W - 8) / st->cat_count;
            render_draw_rect_outline(4 + st->tab_cursor * tab_w - 1, -1,
                                     tab_w + 2, 42, 2.0f, C_BLUE);
        }

        nav_draw(0);
    }
}

static const SceneVTable s_home_vtable = {
    .name = "home",
    .on_enter  = home_on_enter,
    .on_exit   = NULL,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = home_update,
    .draw      = home_draw,
};

static Scene s_home_scene = {
    .vtable = &s_home_vtable,
    .state  = NULL,
};

Scene* scene_home_get(void) {
    return &s_home_scene;
}
