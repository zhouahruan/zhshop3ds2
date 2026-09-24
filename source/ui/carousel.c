/* Carousel — banner image auto-rotator implementation. */
#include "carousel.h"
#include "../core/utils.h"
#include <citro2d.h>
#include <string.h>

static void carousel_on_draw(Widget* self) {
    Carousel* c = (Carousel*)self;
    if (c->count <= 0 || !c->images) return;
    C2D_Image* img = &c->images[c->current];
    if (!img->tex) return;
    C2D_DrawImageAt(*img, c->base.x, c->base.y, 0.5f, NULL, 1.0f, 1.0f);
    if (c->count <= 1) return;
    float dot = 4.0f;
    float gap = 4.0f;
    float total = (float)c->count * dot + (float)(c->count - 1) * gap;
    float sx = c->base.x + (c->base.w - total) * 0.5f;
    float sy = c->base.y + c->base.h - dot - 4.0f;
    for (int i = 0; i < c->count; i++) {
        float dx = sx + (float)i * (dot + gap);
        u32 col = (i == c->current) ? C_YELLOW : C_MUTED;
        render_draw_rounded_rect(dx, sy, dot, dot, dot * 0.5f, col);
    }
}

static int carousel_on_touch(Widget* self, const TouchState* ts) {
    Carousel* c = (Carousel*)self;
    if (!c->base.enabled || c->count <= 0) return 0;
    if (ts->touch_released && widget_contains(&c->base, ts->tx, ts->ty)) {
        float half = c->base.x + c->base.w * 0.5f;
        if (ts->tx < half) {
            c->current = (c->current > 0) ? c->current - 1 : c->count - 1;
        } else {
            c->current = (c->current + 1) % c->count;
        }
        c->last_switch_ms = 0;
        return 1;
    }
    return 0;
}

static const WidgetVTable carousel_vtable = {
    .on_touch = carousel_on_touch,
    .on_draw = carousel_on_draw,
};

void carousel_init(Carousel* c, float x, float y, float w, float h) {
    widget_init(&c->base, x, y, w, h);
    c->base.vtable = &carousel_vtable;
    c->images = NULL;
    c->count = 0;
    c->current = 0;
    c->last_switch_ms = 0;
    c->interval_ms = 3000;
    c->auto_play = 1;
}

void carousel_set_images(Carousel* c, C2D_Image* imgs, int count) {
    c->images = imgs;
    c->count = count;
    c->current = 0;
    c->last_switch_ms = 0;
}

void carousel_update(Carousel* c, uint64_t now_ms) {
    if (!c->auto_play || c->count <= 1) return;
    if (c->last_switch_ms == 0) {
        c->last_switch_ms = now_ms;
        return;
    }
    if (now_ms - c->last_switch_ms >= c->interval_ms) {
        c->current = (c->current + 1) % c->count;
        c->last_switch_ms = now_ms;
    }
}
