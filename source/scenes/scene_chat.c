#include "scene_chat.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "../net/api.h"
#include "../ui/list.h"
#include "../ui/searchbar.h"
#include "nav.h"

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    List session_list;
    List message_list;
    SearchBar input;
    int session_cursor;
    uint8_t loaded;
} SceneChatState;

static const char* chat_session_label(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.sessions.count) return NULL;
    return g_store.sessions.items[idx].display_name;
}

static const char* chat_message_label(int idx, void* ud) {
    (void)ud;
    if (idx < 0 || idx >= g_store.messages.count) return NULL;
    return g_store.messages.items[idx].content;
}

static void chat_load_messages(SceneChatState* st) {
    api.get_messages(g_store.current_session_id, &g_store.messages);
    list_set_data(&st->message_list, g_store.messages.count, chat_message_label, NULL);
}

static void chat_on_select_session(int idx, void* ud) {
    SceneChatState* st = (SceneChatState*)ud;
    if (idx < 0 || idx >= g_store.sessions.count) return;
    utils_strlcpy(g_store.current_session_id, g_store.sessions.items[idx].session_id,
                  sizeof(g_store.current_session_id));
    if (st) chat_load_messages(st);
}

static void chat_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(SceneChatState));
    SceneChatState* st = (SceneChatState*)self->state;
    store_set_route("chat");
    if (!st->loaded) {
        api.get_chat_sessions(&g_store.sessions);
        if (g_store.sessions.count > 0 && !g_store.current_session_id[0]) {
            utils_strlcpy(g_store.current_session_id, g_store.sessions.items[0].session_id,
                          sizeof(g_store.current_session_id));
        }
        st->loaded = 1;
    }
    if (g_store.current_session_id[0]) chat_load_messages(st);
    list_init(&st->session_list, 4, 4, 150, 192, 28, chat_on_select_session, st);
    list_set_data(&st->session_list, g_store.sessions.count, chat_session_label, NULL);
    list_init(&st->message_list, 4, 4, 392, 224, 24, NULL, NULL);
    list_set_data(&st->message_list, g_store.messages.count, chat_message_label, NULL);
    searchbar_init(&st->input, 160, 168, 152, 28, NULL, st);
    st->session_cursor = 0;
}

static void chat_update(Scene* self) {
    SceneChatState* st = (SceneChatState*)self->state;
    if (!st) return;

    /* Touch input. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (nav_handle_touch(t)) return;
        if (widget_touch(&st->session_list.base, t)) return;
        widget_touch(&st->input.base, t);
        return;
    }

    /* Gamepad navigation. */
    if (nav_handle_gamepad()) return;

    uint8_t nav = input_nav_pressed();
    int total = g_store.sessions.count;
    if (total > 0) {
        if (nav & NAV_UP)   { st->session_cursor--; if (st->session_cursor < 0) st->session_cursor = 0; }
        if (nav & NAV_DOWN) { st->session_cursor++; if (st->session_cursor >= total) st->session_cursor = total - 1; }
        if (input_pressed(KEY_A_3DS)) {
            chat_on_select_session(st->session_cursor, st);
        }
    }
    if (input_pressed(KEY_B_3DS)) {
        extern Scene* scene_home_get(void);
        scene_manager_switch(scene_home_get(), NULL);
    }
}

static void chat_draw(Scene* self, ScreenId screen) {
    SceneChatState* st = (SceneChatState*)self->state;
    if (!st) return;
    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);
        render_draw_text(8, 4, 0.8f, 0.8f, C_TEXT, "Chat");
        if (g_store.messages.count <= 0) {
            render_draw_text(8, 100, 0.7f, 0.7f, C_MUTED, "No messages yet.");
        } else {
            int visible = 9;
            int start = g_store.messages.count - visible;
            if (start < 0) start = 0;
            float y = 24;
            for (int i = start; i < g_store.messages.count && y < 232; ++i) {
                Message* m = &g_store.messages.items[i];
                u32 col = m->from_self ? C_BLUE : C_TEXT;
                const char* prefix = m->from_self ? "Me: " : "";
                render_draw_text(8, y, 0.55f, 0.55f, col, "%s%s", prefix, m->content);
                y += 22;
            }
        }
        render_draw_text(8, TOP_H - 14, 0.5f, 0.5f, C_MUTED,
            "Up/Down: select session  |  A: load  |  B: back");
    } else {
        render_clear(C_SCREEN_BG);
        widget_draw(&st->session_list.base);
        const char* name = "";
        for (int i = 0; i < g_store.sessions.count; ++i) {
            if (strcmp(g_store.sessions.items[i].session_id, g_store.current_session_id) == 0) {
                name = g_store.sessions.items[i].display_name;
                break;
            }
        }
        if (name[0])
            render_draw_text(160, 8, 0.6f, 0.6f, C_MUTED, "To: %s", name);
        widget_draw(&st->input.base);

        /* Gamepad cursor highlight on session list. */
        if (g_store.sessions.count > 0) {
            float item_y = 4 + st->session_cursor * 28.0f;
            render_draw_rect_outline(3, item_y - 1, 152, 30, 2.0f, C_BLUE);
        }

        nav_draw(4);
    }
}

static void chat_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static const SceneVTable s_chat_vtable = {
    .name = "chat",
    .on_enter  = chat_on_enter,
    .on_exit   = chat_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = chat_update,
    .draw      = chat_draw,
};

static Scene s_chat_scene = {
    .vtable = &s_chat_vtable,
    .state  = NULL,
};

Scene* scene_chat_get(void) {
    return &s_chat_scene;
}
