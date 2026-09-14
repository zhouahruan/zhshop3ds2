#include "nav.h"
#include "../core/render.h"
#include "../core/utils.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "scene_home.h"
#include "scene_applist.h"
#include "scene_downloads.h"
#include "scene_forum.h"
#include "scene_chat.h"
#include "scene_settings.h"

#include <3ds.h>
#include <stdio.h>

/* Nav bar geometry on bottom screen. */
#define NAV_X         4.0f
#define NAV_Y         200.0f
#define NAV_W         (BOTTOM_W - 8.0f)
#define NAV_H         36.0f
#define NAV_GAP       4.0f
#define NAV_BTN_W     ((NAV_W - 4 * NAV_GAP) / 5.0f)

typedef struct { const char* label; Scene* (*get)(void); } NavItem;
static const NavItem NAV_ITEMS[5] = {
    { "Home",     scene_home_get     },
    { "Apps",     scene_applist_get  },
    { "Downloads",scene_downloads_get},
    { "Forum",    scene_forum_get    },
    { "Chat",     scene_chat_get     },
};

/* Track which nav button is focused for gamepad navigation. */
static int s_nav_focus = 0;

int nav_handle_touch(const TouchState* t) {
    if (!t || !t->touch_pressed) return 0;
    if (t->ty < NAV_Y || t->ty > NAV_Y + NAV_H) return 0;
    for (int i = 0; i < 5; ++i) {
        float bx = NAV_X + i * (NAV_BTN_W + NAV_GAP);
        if (t->tx >= bx && t->tx <= bx + NAV_BTN_W) {
            s_nav_focus = i;
            Scene* s = NAV_ITEMS[i].get();
            scene_manager_switch(s, NULL);
            return 1;
        }
    }
    return 0;
}

/* Handle gamepad left/right to move focus, A to activate. Returns 1 if
 * consumed. Called from every scene's update before its own widgets. */
int nav_handle_gamepad(void) {
    uint8_t nav = input_nav_pressed();
    if (nav & NAV_LEFT) {
        if (s_nav_focus > 0) s_nav_focus--;
        else s_nav_focus = 4;
        return 1;
    }
    if (nav & NAV_RIGHT) {
        s_nav_focus = (s_nav_focus + 1) % 5;
        return 1;
    }
    if (input_pressed(KEY_A_3DS)) {
        Scene* s = NAV_ITEMS[s_nav_focus].get();
        scene_manager_switch(s, NULL);
        return 1;
    }
    return 0;
}

int nav_focus_index(void) { return s_nav_focus; }

void nav_set_focus(int idx) {
    if (idx >= 0 && idx < 5) s_nav_focus = idx;
}

void nav_draw(int active_index) {
    /* Only update focus on first draw or if no focus set yet. */
    render_draw_rect(NAV_X - 2, NAV_Y - 2, NAV_W + 4, NAV_H + 4, C_BEZEL);
    for (int i = 0; i < 5; ++i) {
        float bx = NAV_X + i * (NAV_BTN_W + NAV_GAP);
        u32 bg = (i == active_index) ? C_BLUE : C_SURFACE;
        u32 fg = (i == active_index) ? C_SURFACE : C_TEXT;
        render_draw_rounded_rect(bx, NAV_Y, NAV_BTN_W, NAV_H, 6.0f, bg);
        float sx = 0.55f, sy = 0.55f;
        const char* label = NAV_ITEMS[i].label;
        float tw = render_text_width(sx, label);
        render_draw_text(bx + (NAV_BTN_W - tw) * 0.5f,
                         NAV_Y + (NAV_H - 8.0f * sy) * 0.5f,
                         sx, sy, fg, "%s", label);
    }
}
