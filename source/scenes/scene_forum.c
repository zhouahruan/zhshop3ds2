#include "scene_forum.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../net/api.h"
#include "../ui/list.h"
#include "../ui/button.h"
#include "../ui/modal.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    List   post_list;
    Button new_post_btn;
    Modal  modal;
    int    list_cursor;
    uint8_t loaded;
} SceneForumState;

static const char* forum_post_label(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.posts.count) return NULL;
    return g_store.posts.items[idx].title;
}

static void forum_refresh(SceneForumState* st) {
    api.get_post_list(1, &g_store.posts);
    list_set_data(&st->post_list, g_store.posts.count, forum_post_label, NULL);
}

static void forum_on_select_post(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.posts.count) return;
    Post* p = &g_store.posts.items[idx];
    api.get_post_detail(p->id, &g_store.current_post, &g_store.replies);
}

static void forum_on_post_submitted(int ok, const char* text, void* ud) {
    SceneForumState* st = (SceneForumState*)ud;
    if (!ok || !text || !text[0]) return;
    api.create_post(text, "");
    forum_refresh(st);
}

static void forum_on_new_post(void* ud) {
    SceneForumState* st = (SceneForumState*)ud;
    modal_show_input(&st->modal, "New Post", "Title", forum_on_post_submitted, st);
}

static void forum_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneForumState));
    SceneForumState* st = (SceneForumState*)self->state;
    store_set_route("forum");
    if (!st->loaded) forum_refresh(st);
    list_init(&st->post_list, 4, 4, 312, 160, 28, forum_on_select_post, st);
    list_set_data(&st->post_list, g_store.posts.count, forum_post_label, NULL);
    button_init(&st->new_post_btn, 4, 168, 100, 28, "New Post", C_BLUE, C_SURFACE,
                forum_on_new_post, st);
    modal_init(&st->modal);
    st->list_cursor = 0;
    st->loaded = 1;
}

static void forum_update(Scene* self) {
    SceneForumState* st = (SceneForumState*)self->state;
    if (!st) return;

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (modal_handle_touch(&st->modal, t)) return;
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->new_post_btn.base, t)) return;
        widget_touch(&st->post_list.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    uint8_t nav = input_nav_pressed();
    int total = g_store.posts.count;
    if (total > 0) {
        if (nav & NAV_UP)   { st->list_cursor--; if (st->list_cursor < 0) st->list_cursor = 0; }
        if (nav & NAV_DOWN) { st->list_cursor++; if (st->list_cursor >= total) st->list_cursor = total - 1; }
        if (input_pressed(KEY_A_3DS)) {
            forum_on_select_post(st->list_cursor, NULL);
        }
    }
    if (input_pressed(KEY_B_3DS)) {
        extern Scene* scene_home_get(void);
        scene_manager_switch(scene_home_get(), NULL);
    }
}

static void forum_draw(Scene* self, ScreenId screen) {
    SceneForumState* st = (SceneForumState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "Forum");
        render_draw_text(8, 24, 0.6f, 0.6f, C_MUTED, "%d posts", g_store.posts.count);

        /* Show selected post detail on top screen. */
        if (g_store.posts.count > 0 && st->list_cursor < g_store.posts.count) {
            Post* p = &g_store.posts.items[st->list_cursor];
            render_draw_text(8, 44, 0.7f, 0.7f, C_TEXT, "%s", p->title);
            render_draw_text(8, 62, 0.5f, 0.5f, C_MUTED, "by %s", p->author);
            const char* desc = p->body;
            float sx = 0.5f, sy = 0.5f;
            int max_chars = (int)((TOP_W - 16) / (8.0f * sx));
            if (max_chars < 8) max_chars = 8;
            float y = 80;
            int len = (int)strlen(desc);
            int pos = 0;
            while (pos < len && y < 230) {
                int chunk = max_chars;
                if (chunk > 255) chunk = 255;
                if (pos + chunk > len) chunk = len - pos;
                char buf[256];
                utils_strlcpy(buf, desc + pos, (size_t)chunk + 1);
                render_draw_text(8, y, sx, sy, C_TEXT, "%s", buf);
                y += 10.0f * sy;
                pos += chunk;
            }
        }

        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "Up/Down: navigate  |  A: select  |  B: back");
        modal_draw_top(&st->modal);
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->post_list.base);
        widget_draw(&st->new_post_btn.base);

        /* Gamepad cursor highlight. */
        if (g_store.posts.count > 0) {
            float item_y = 4 + st->list_cursor * 28.0f;
            render_draw_rect_outline(3, item_y - 1, 314, 30, 2.0f, C_BLUE);
        }

        nav_draw(3);
        modal_draw_bottom(&st->modal);
    }
}

static void forum_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static const SceneVTable s_forum_vtable = {
    .name = "forum",
    .on_enter  = forum_on_enter,
    .on_exit   = forum_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = forum_update,
    .draw      = forum_draw,
};

static Scene s_forum_scene = {
    .vtable = &s_forum_vtable,
    .state  = NULL,
};

Scene* scene_forum_get(void) {
    return &s_forum_scene;
}
