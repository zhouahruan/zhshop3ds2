#include "scene_applist.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../net/api.h"
#include "../ui/grid.h"
#include "../ui/searchbar.h"
#include "../ui/tabs.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Grid      grid;
    SearchBar searchbar;
    Tabs      sort_tabs;
    int      grid_cursor;
    uint8_t  loaded;
} SceneAppListState;

static const char* SORT_LABELS[3] = { "Hot", "New", "Top Rated" };
static const char* SORT_VALUES[3] = { "download_count", "created_at", "avg_rating" };

static const char** applist_grid_label(int idx, void* ud) {
    (void)ud;
    static const char* slot = NULL;
    if (idx < 0 || idx >= g_store.current_list.count) { slot = NULL; return &slot; }
    slot = g_store.current_list.items[idx].name;
    return &slot;
}

static void applist_open_detail(int idx) {
    if (idx < 0 || idx >= g_store.current_list.count) return;
    App* app = &g_store.current_list.items[idx];
    if (g_store.current_detail) { free(g_store.current_detail); g_store.current_detail = NULL; }
    g_store.current_detail = calloc(1, sizeof(AppDetail));
    if (!g_store.current_detail) return;
    api.get_app_detail(app->id, g_store.current_detail);
    extern Scene* scene_detail_get(void);
    scene_manager_switch(scene_detail_get(), NULL);
}

static void applist_on_select_grid(int idx, void* ud) {
    (void)ud;
    applist_open_detail(idx);
}

static int applist_current_sort_index(void) {
    for (int i = 0; i < 3; ++i)
        if (strcmp(g_store.list_sort, SORT_VALUES[i]) == 0) return i;
    return 0;
}

static void applist_refresh(SceneAppListState* st) {
    api.get_app_list(g_store.list_category, g_store.list_keyword,
                     g_store.list_sort,
                     NULL, -1, NULL,
                     g_store.list_page, 20, &g_store.current_list);
    grid_set_data(&st->grid, g_store.current_list.count, applist_grid_label, NULL);
}

static void applist_on_submit_search(const char* text, void* ud) {
    SceneAppListState* st = (SceneAppListState*)ud;
    utils_strlcpy(g_store.list_keyword, text ? text : "", sizeof(g_store.list_keyword));
    g_store.list_page = 1;
    applist_refresh(st);
}

static void applist_on_change_sort(int new_index, void* ud) {
    SceneAppListState* st = (SceneAppListState*)ud;
    if (new_index < 0 || new_index >= 3) return;
    utils_strlcpy(g_store.list_sort, SORT_VALUES[new_index], sizeof(g_store.list_sort));
    g_store.list_page = 1;
    applist_refresh(st);
}

static void applist_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneAppListState));
    SceneAppListState* st = (SceneAppListState*)self->state;
    store_set_route("applist");
    if (!st->loaded || g_store.loading) {
        g_store.loading = 0;
        applist_refresh(st);
    }
    searchbar_init(&st->searchbar, 8, 8, 304, 28, applist_on_submit_search, st);
    tabs_init(&st->sort_tabs, 4, 40, 312, 32, SORT_LABELS, 3,
              applist_current_sort_index(), C_BLUE, C_SURFACE, C_TEXT,
              applist_on_change_sort, st);
    grid_init(&st->grid, 4, 76, 312, 120, 4, 4, 4, applist_on_select_grid, NULL);
    grid_set_data(&st->grid, g_store.current_list.count, applist_grid_label, NULL);
    st->grid_cursor = 0;
    st->loaded = 1;
}

static void applist_update(Scene* self) {
    SceneAppListState* st = (SceneAppListState*)self->state;
    if (!st) return;

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->searchbar.base, t)) return;
        if (widget_touch(&st->sort_tabs.base, t)) return;
        widget_touch(&st->grid.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    uint8_t nav = input_nav_pressed();
    int total = g_store.current_list.count;
    if (total <= 0) return;
    int cols = 4;
    if (nav & NAV_LEFT)  { st->grid_cursor--; if (st->grid_cursor < 0) st->grid_cursor = total - 1; }
    if (nav & NAV_RIGHT) { st->grid_cursor = (st->grid_cursor + 1) % total; }
    if (nav & NAV_UP)    { st->grid_cursor -= cols; if (st->grid_cursor < 0) st->grid_cursor = 0; }
    if (nav & NAV_DOWN)  { st->grid_cursor += cols; if (st->grid_cursor >= total) st->grid_cursor = total - 1; }
    if (input_pressed(KEY_A_3DS)) {
        applist_open_detail(st->grid_cursor);
    }
    if (input_pressed(KEY_B_3DS)) {
        extern Scene* scene_home_get(void);
        scene_manager_switch(scene_home_get(), NULL);
    }
}

static void applist_draw(Scene* self, ScreenId screen) {
    SceneAppListState* st = (SceneAppListState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "Apps");
        if (g_store.list_category[0])
            render_draw_text(8, 24, 0.6f, 0.6f, C_MUTED, "category: %s", g_store.list_category);
        if (g_store.list_keyword[0])
            render_draw_text(8, 40, 0.6f, 0.6f, C_MUTED, "search: %s", g_store.list_keyword);

        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "D-pad/C-pad: navigate  |  A: open  |  B: back");
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->searchbar.base);
        widget_draw(&st->sort_tabs.base);
        widget_draw(&st->grid.base);

        /* Gamepad cursor highlight. */
        if (g_store.current_list.count > 0) {
            float cw = (312.0f - 4 * 4) / 4.0f;
            float ch = (120.0f - 3 * 4) / 4.0f;
            int col = st->grid_cursor % 4;
            int row = st->grid_cursor / 4;
            float cx = 4 + col * (cw + 4);
            float cy = 76 + row * (ch + 4);
            render_draw_rect_outline(cx - 1, cy - 1, cw + 2, ch + 2, 2.0f, C_BLUE);
        }

        nav_draw(1);
    }
}

static void applist_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static const SceneVTable s_applist_vtable = {
    .name = "applist",
    .on_enter  = applist_on_enter,
    .on_exit   = applist_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = applist_update,
    .draw      = applist_draw,
};

static Scene s_applist_scene = {
    .vtable = &s_applist_vtable,
    .state  = NULL,
};

Scene* scene_applist_get(void) {
    return &s_applist_scene;
}
