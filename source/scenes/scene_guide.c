/**
 * Onboarding guide scene — multi-page tutorial that teaches the user
 * both touch and gamepad controls. Shown on first launch (before home).
 *
 * Pages:
 *   0: Welcome
 *   1: Touch controls (tap apps to open, tap nav bar to switch tabs)
 *   2: Gamepad controls (D-pad/C-pad to navigate, A to select, B to back)
 *   3: Ready to start
 *
 * Advance with A button, touch tap, or D-pad right. Go back with B or
 * D-pad left. Start on last page goes to home.
 */
#include "scene_guide.h"
#include "../core/utils.h"
#include "../core/render.h"
#include "../core/input.h"
#include "../core/scene_manager.h"
#include "../data/store.h"
#include "scene_home.h"

#include <3ds.h>
#include <stdlib.h>

#define GUIDE_PAGES 4

typedef struct {
    int page;
} GuideState;

static void guide_on_enter(Scene* self, void* args) {
    (void)args;
    if (!self->state) self->state = calloc(1, sizeof(GuideState));
    GuideState* st = (GuideState*)self->state;
    st->page = 0;
    store_set_route("guide");
}

static void guide_on_exit(Scene* self) {
    if (self->state) {
        free(self->state);
        self->state = NULL;
    }
}

static void guide_go_home(void) {
    scene_manager_switch(scene_home_get(), NULL);
}

static void guide_update(Scene* self) {
    GuideState* st = (GuideState*)self->state;
    if (!st) return;

    uint8_t nav = input_nav_pressed();

    /* Next page. */
    if (input_pressed(KEY_A_3DS) || (nav & NAV_RIGHT) ||
        (nav & NAV_DOWN)) {
        if (st->page >= GUIDE_PAGES - 1) {
            guide_go_home();
            return;
        }
        st->page++;
        return;
    }

    /* Previous page. */
    if (input_pressed(KEY_B_3DS) || (nav & NAV_LEFT) ||
        (nav & NAV_UP)) {
        if (st->page <= 0) {
            guide_go_home();
            return;
        }
        st->page--;
        return;
    }

    /* Touch to advance. */
    const TouchState* t = input_touch();
    if (t && t->touch_pressed) {
        if (st->page >= GUIDE_PAGES - 1) {
            guide_go_home();
            return;
        }
        st->page++;
        return;
    }

    /* Start button always skips to home. */
    if (input_pressed(KEY_START_3DS)) {
        guide_go_home();
    }
}

static void guide_draw(Scene* self, ScreenId screen) {
    GuideState* st = (GuideState*)self->state;
    if (!st) return;
    int page = st->page;

    if (screen == SCREEN_TOP) {
        render_clear(C_SCREEN_BG);

        /* Page content. */
        switch (page) {
        case 0:
            /* Welcome page. */
            render_draw_rounded_rect(40, 30, 320, 180, 12.0f, C_BLUE);
            render_draw_text(100, 60, 1.2f, 1.2f, C_SURFACE, "3DS App Store");
            render_draw_text(80, 100, 0.6f, 0.6f, C_SURFACE,
                             "Your homebrew app store for Nintendo 3DS");
            render_draw_text(90, 130, 0.6f, 0.6f, C_SURFACE,
                             "Browse, download and install homebrew");
            render_draw_text(110, 150, 0.6f, 0.6f, C_SURFACE,
                             "CIA apps directly on your console.");
            break;

        case 1:
            /* Touch controls page. */
            render_draw_text(20, 16, 0.9f, 0.9f, C_TEXT, "Touch Controls");
            render_draw_rect(20, 36, 360, 1, C_MUTED);

            render_draw_text(20, 50, 0.6f, 0.6f, C_TEXT,
                "Tap an app icon to open its detail page.");
            render_draw_text(20, 70, 0.6f, 0.6f, C_TEXT,
                "Tap the bottom nav bar to switch between");
            render_draw_text(20, 84, 0.6f, 0.6f, C_TEXT,
                "Home, Apps, Downloads, Forum and Chat.");
            render_draw_text(20, 108, 0.6f, 0.6f, C_TEXT,
                "Use the search bar on the Apps page to find");
            render_draw_text(20, 122, 0.6f, 0.6f, C_TEXT,
                "apps by keyword.");
            render_draw_text(20, 146, 0.6f, 0.6f, C_TEXT,
                "Tap the Download button on any app's");
            render_draw_text(20, 160, 0.6f, 0.6f, C_TEXT,
                "detail page to start downloading.");

            /* Visual: stylus icon. */
            render_draw_rounded_rect(280, 180, 80, 40, 6.0f, C_BLUE);
            render_draw_text(296, 192, 0.7f, 0.7f, C_SURFACE, "Touch");
            break;

        case 2:
            /* Gamepad controls page. */
            render_draw_text(20, 16, 0.9f, 0.9f, C_TEXT, "Gamepad Controls");
            render_draw_rect(20, 36, 360, 1, C_MUTED);

            render_draw_text(20, 50, 0.6f, 0.6f, C_TEXT,
                "D-pad or Circle Pad: navigate lists and grids.");
            render_draw_text(20, 70, 0.6f, 0.6f, C_TEXT,
                "A button: select / confirm.");
            render_draw_text(20, 88, 0.6f, 0.6f, C_TEXT,
                "B button: go back.");
            render_draw_text(20, 106, 0.6f, 0.6f, C_TEXT,
                "L/R buttons: switch between panels");
            render_draw_text(20, 120, 0.6f, 0.6f, C_TEXT,
                "(e.g. grid vs tabs on the Home screen).");
            render_draw_text(20, 144, 0.6f, 0.6f, C_TEXT,
                "Start+Select: force quit the app.");

            /* Visual: button diagram. */
            render_draw_rounded_rect(150, 170, 30, 24, 4.0f, C_SURFACE);
            render_draw_text(156, 176, 0.6f, 0.6f, C_TEXT, "A");
            render_draw_rounded_rect(110, 170, 30, 24, 4.0f, C_SURFACE);
            render_draw_text(116, 176, 0.6f, 0.6f, C_TEXT, "B");
            break;

        case 3:
            /* Ready to start. */
            render_draw_rounded_rect(40, 30, 320, 180, 12.0f, C_BLUE);
            render_draw_text(120, 60, 1.0f, 1.0f, C_SURFACE, "You're Ready!");
            render_draw_text(70, 100, 0.6f, 0.6f, C_SURFACE,
                             "Press A or tap the screen to enter the store.");
            render_draw_text(80, 130, 0.6f, 0.6f, C_SURFACE,
                             "You can revisit this guide anytime");
            render_draw_text(95, 148, 0.6f, 0.6f, C_SURFACE,
             "from Settings.");
            break;
        }

        /* Progress dots. */
        float dot_y = 224;
        float dot_w = 8;
        float dot_gap = 12;
        float total_w = GUIDE_PAGES * dot_w + (GUIDE_PAGES - 1) * dot_gap;
        float start_x = (TOP_W - total_w) * 0.5f;
        for (int i = 0; i < GUIDE_PAGES; ++i) {
            float x = start_x + i * (dot_w + dot_gap);
            u32 col = (i == page) ? C_BLUE : C_MUTED;
            render_draw_rounded_rect(x, dot_y, dot_w, dot_w, 4.0f, col);
        }
    } else {
        /* Bottom screen: page navigation hints. */
        render_clear(C_SCREEN_BG);

        float bw = 120, bh = 36;
        float bx = (BOTTOM_W - bw) * 0.5f;
        float by = (BOTTOM_H - bh) * 0.5f;

        const char* btn_label = (page >= GUIDE_PAGES - 1) ? "Start" : "Next";

        render_draw_rounded_rect(bx, by, bw, bh, 6.0f, C_BLUE);
        float tw = render_text_width(0.7f, btn_label);
        render_draw_text(bx + (bw - tw) * 0.5f, by + 10, 0.7f, 0.7f,
                         C_SURFACE, "%s", btn_label);

        render_draw_text(8, BOTTOM_H - 14, 0.5f, 0.5f, C_MUTED,
            "A/tap: next  |  B: back  |  Start: skip");
    }
}

static const SceneVTable s_guide_vtable = {
    .name = "guide",
    .on_enter  = guide_on_enter,
    .on_exit   = guide_on_exit,
    .on_pause  = NULL,
    .on_resume = NULL,
    .update    = guide_update,
    .draw      = guide_draw,
};

static Scene s_guide_scene = {
    .vtable = &s_guide_vtable,
    .state  = NULL,
};

Scene* scene_guide_get(void) {
    return &s_guide_scene;
}
