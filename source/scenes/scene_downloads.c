#include "scene_downloads.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../ui/progressbar.h"
#include "../ui/list.h"
#include "../ui/tabs.h"
#include "nav.h"

#if USE_MOCK
#include "../net/api_config.h"
#else
#include "../download/downloader.h"
#endif

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    List task_list;
    List installed_list;
    Tabs mode_tabs;
    ProgressBar bars[MAX_DOWNLOADS];
    int list_cursor;
    uint8_t loaded;
    int mode;
} SceneDownloadsState;

static const char* DL_MODE_LABELS[2] = { "Tasks", "Installed" };

static const char* downloads_task_label(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.download_count) return NULL;
    return g_store.downloads[idx].app_name;
}

static const char* downloads_installed_label(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.installed_count) return NULL;
    return g_store.installed[idx].name;
}

static void downloads_on_change_mode(int new_index, void* ud) {
    SceneDownloadsState* st = (SceneDownloadsState*)ud;
    if (new_index < 0 || new_index > 1) return;
    st->mode = new_index;
    st->list_cursor = 0;
}

static void downloads_tick(void) {
#if USE_MOCK
    for (int i = 0; i < g_store.download_count; ++i) {
        DownloadTask* t = &g_store.downloads[i];
        if (t->status == DL_DONE || t->status == DL_FAILED) continue;
        if (t->status == DL_PENDING) t->status = DL_RUNNING;
        if (t->status == DL_RUNNING) {
            t->progress += 2;
            if (t->progress >= 100) {
                t->progress = 100;
                t->status = DL_DONE;
            }
        }
    }
#else
    downloader_tick();
#endif
}

static void downloads_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneDownloadsState));
    SceneDownloadsState* st = (SceneDownloadsState*)self->state;
    store_set_route("downloads");
    st->loaded = 1;
    list_init(&st->task_list, 4, 44, 312, 152, 28, NULL, NULL);
    list_set_data(&st->task_list, g_store.download_count, downloads_task_label, NULL);
    list_init(&st->installed_list, 4, 20, 392, 212, 28, NULL, NULL);
    list_set_data(&st->installed_list, g_store.installed_count,
                  downloads_installed_label, NULL);
    tabs_init(&st->mode_tabs, 4, 8, 312, 32, DL_MODE_LABELS, 2, st->mode,
              C_BLUE, C_SURFACE, C_TEXT, downloads_on_change_mode, st);
    for (int i = 0; i < MAX_DOWNLOADS; ++i) {
        progressbar_init(&st->bars[i], 8, 8 + (float)i * 32.0f, 380, 24,
                         C_BLUE, C_TEXT);
    }
    st->list_cursor = 0;
}

static void downloads_update(Scene* self) {
    SceneDownloadsState* st = (SceneDownloadsState*)self->state;
    if (!st) return;
    downloads_tick();

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->mode_tabs.base, t)) return;
        if (st->mode == 0) widget_touch(&st->task_list.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    /* L/R buttons to switch mode tabs (Tasks/Installed). */
    if (input_pressed(KEY_L_3DS)) { st->mode = 0; st->list_cursor = 0; return; }
    if (input_pressed(KEY_R_3DS)) { st->mode = 1; st->list_cursor = 0; return; }

    uint8_t nav = input_nav_pressed();

    /* Up/Down to navigate list. */
    int total = (st->mode == 0) ? g_store.download_count : g_store.installed_count;
    if (total > 0) {
        if (nav & NAV_UP)   { st->list_cursor--; if (st->list_cursor < 0) st->list_cursor = 0; }
        if (nav & NAV_DOWN) { st->list_cursor++; if (st->list_cursor >= total) st->list_cursor = total - 1; }
    }
}

static void downloads_draw(Scene* self, ScreenId screen) {
    SceneDownloadsState* st = (SceneDownloadsState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        if (st->mode == 0) {
            if (g_store.download_count <= 0) {
                render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "Downloads");
                render_draw_text(8, 120, 0.7f, 0.7f, C_MUTED, "No download tasks.");
            }
            for (int i = 0; i < g_store.download_count; ++i) {
                DownloadTask* t = &g_store.downloads[i];
                progressbar_set(&st->bars[i], t->progress, t->app_name);
                widget_draw(&st->bars[i].base);
            }
        } else {
            render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "Installed");
            widget_draw(&st->installed_list.base);
        }
        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "L/R or D-pad: switch mode  |  Up/Down: navigate list");
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->mode_tabs.base);
        if (st->mode == 0) widget_draw(&st->task_list.base);

        /* Gamepad cursor highlight on list. */
        int total = (st->mode == 0) ? g_store.download_count : g_store.installed_count;
        if (total > 0) {
            float item_y = 44 + st->list_cursor * 28.0f;
            render_draw_rect_outline(3, item_y - 1, 314, 30, 2.0f, C_BLUE);
        }

        nav_draw(2);
    }
}

static void downloads_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static const SceneVTable s_downloads_vtable = {
    .name = "downloads",
    .on_enter  = downloads_on_enter,
    .on_exit   = downloads_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = downloads_update,
    .draw      = downloads_draw,
};

static Scene s_downloads_scene = {
    .vtable = &s_downloads_vtable,
    .state  = NULL,
};

Scene* scene_downloads_get(void) {
    return &s_downloads_scene;
}
