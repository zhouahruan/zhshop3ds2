#include "scene_detail.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../net/api.h"
#include "../ui/button.h"
#include "../ui/rating.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Button back_btn;
    Button download_btn;
    Rating rating;
    int cursor;       /* 0=back, 1=download */
    uint8_t loaded;
} SceneDetailState;

static void detail_on_back(void* ud) {
    (void)ud;
    scene_manager_pop();
}

static void detail_on_download(void* ud) {
    (void)ud;
    if (!g_store.current_detail) return;
    App* app = &g_store.current_detail->base;
    char url[URL_LEN];
    int size = 0;
    api.get_download_url(app->id, url, sizeof(url), &size);
    int idx = store_add_download(app);
    if (idx >= 0) {
        utils_strlcpy(g_store.downloads[idx].download_url, url,
                      sizeof(g_store.downloads[idx].download_url));
        g_store.downloads[idx].total_bytes = size > 0 ? size : app->package_size;
        g_store.downloads[idx].status = DL_PENDING;
        g_store.downloads[idx].progress = 0;
    }
    extern Scene* scene_downloads_get(void);
    scene_manager_switch(scene_downloads_get(), NULL);
}

static void detail_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneDetailState));
    SceneDetailState* st = (SceneDetailState*)self->state;
    store_set_route("detail");
    st->loaded = (g_store.current_detail != NULL) ? 1 : 0;
    button_init(&st->back_btn, 8, 8, 80, 30, "Back", C_MUTED, C_SURFACE,
                detail_on_back, NULL);
    button_init(&st->download_btn, 96, 8, 120, 30, "Download", C_BLUE, C_SURFACE,
                detail_on_download, NULL);
    float rating_val = (st->loaded && g_store.current_detail)
                       ? g_store.current_detail->base.avg_rating : 0.0f;
    rating_init(&st->rating, 8, 178, 16.0f, rating_val, C_YELLOW, 0, NULL, NULL);
    st->cursor = 1;
}

static void detail_update(Scene* self) {
    SceneDetailState* st = (SceneDetailState*)self->state;
    if (!st) return;

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->back_btn.base, t)) return;
        widget_touch(&st->download_btn.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    uint8_t nav = input_nav_pressed();
    if (nav & NAV_LEFT)  { st->cursor = 0; }
    if (nav & NAV_RIGHT) { st->cursor = 1; }
    if (input_pressed(KEY_A_3DS)) {
        if (st->cursor == 0) detail_on_back(NULL);
        else                 detail_on_download(NULL);
    }
    if (input_pressed(KEY_B_3DS)) {
        detail_on_back(NULL);
    }
}

static void detail_draw(Scene* self, ScreenId screen) {
    SceneDetailState* st = (SceneDetailState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        if (!st->loaded || !g_store.current_detail) {
            render_draw_text(8, 100, 0.8f, 0.8f, C_MUTED, "No app selected.");
            return;
        }
        App* a = &g_store.current_detail->base;
        render_draw_text(8, 8, 0.9f, 0.9f, C_TEXT, "%s", a->name);
        render_draw_text(8, 30, 0.6f, 0.6f, C_MUTED, "by %s  v%s", a->developer_name, a->version);
        widget_draw(&st->rating.base);

        /* 3DS-specific info. */
        if (a->title_id[0])
            render_draw_text(8, 176, 0.55f, 0.55f, C_MUTED, "Title ID: %s", a->title_id);
        if (a->min_firmware[0])
            render_draw_text(8, 188, 0.55f, 0.55f, C_MUTED, "Min FW: %s", a->min_firmware);

        char bytes[16];
        utils_format_bytes(bytes, sizeof(bytes), (uint64_t)a->package_size);
        render_draw_text(8, 200, 0.6f, 0.6f, C_TEXT, "Downloads: %d   Size: %s",
                         a->download_count, bytes);
        const char* desc = g_store.current_detail->long_description[0]
                           ? g_store.current_detail->long_description
                           : a->description;
        float sx = 0.55f, sy = 0.55f;
        int max_chars = (int)((TOP_W - 16) / (8.0f * sx));
        if (max_chars < 8) max_chars = 8;
        float y = 56;
        int len = (int)strlen(desc);
        int pos = 0;
        while (pos < len && y < 170) {
            int chunk = max_chars;
            if (chunk > 255) chunk = 255;
            if (pos + chunk > len) chunk = len - pos;
            char buf[256];
            utils_strlcpy(buf, desc + pos, (size_t)chunk + 1);
            render_draw_text(8, y, sx, sy, C_TEXT, "%s", buf);
            y += 10.0f * sy;
            pos += chunk;
        }

        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "A: select button  |  B: back");
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->back_btn.base);
        widget_draw(&st->download_btn.base);

        /* Gamepad cursor highlight. */
        float cx = (st->cursor == 0) ? 7.0f : 95.0f;
        float cw = (st->cursor == 0) ? 82.0f : 122.0f;
        render_draw_rect_outline(cx, 7.0f, cw, 32.0f, 2.0f, C_BLUE);

        nav_draw(1);
    }
}

static const SceneVTable s_detail_vtable = {
    .name = "detail",
    .on_enter  = detail_on_enter,
    .on_exit   = NULL,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = detail_update,
    .draw      = detail_draw,
};

static Scene s_detail_scene = {
    .vtable = &s_detail_vtable,
    .state  = NULL,
};

Scene* scene_detail_get(void) {
    return &s_detail_scene;
}
