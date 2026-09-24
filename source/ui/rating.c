/* Rating — star rating implementation. */
#include "rating.h"
#include "../core/utils.h"

static void rating_on_draw(Widget* self) {
    Rating* r = (Rating*)self;
    float sz = r->base.h;
    for (int i = 0; i < 5; i++) {
        float sx = r->base.x + (float)i * sz;
        float sy = r->base.y;
        float v = r->value - (float)i;
        if (v >= 1.0f) {
            render_draw_rounded_rect(sx, sy, sz, sz, 2.0f, r->star_color);
        } else if (v >= 0.5f) {
            render_draw_rounded_rect(sx, sy, sz * 0.5f, sz, 2.0f, r->star_color);
            render_draw_rounded_rect(sx + sz * 0.5f, sy, sz * 0.5f, sz, 2.0f,
                                     r->empty_color);
        } else {
            render_draw_rounded_rect(sx, sy, sz, sz, 2.0f, r->empty_color);
        }
    }
}

static int rating_on_touch(Widget* self, const TouchState* ts) {
    Rating* r = (Rating*)self;
    if (!r->editable || !r->base.enabled) return 0;
    if (ts->touch_pressed && widget_contains(&r->base, ts->tx, ts->ty)) {
        float sz = r->base.h;
        float rel = ts->tx - r->base.x;
        if (rel < 0.0f) rel = 0.0f;
        float units = (rel / sz) * 2.0f;
        int i_units = (int)(units + 0.5f);
        float new_val = (float)i_units * 0.5f;
        if (new_val < 0.0f) new_val = 0.0f;
        if (new_val > 5.0f) new_val = 5.0f;
        r->value = new_val;
        if (r->on_change) r->on_change(new_val, r->base.user_data);
        return 1;
    }
    return 0;
}

static const WidgetVTable rating_vtable = {
    .on_touch = rating_on_touch,
    .on_draw = rating_on_draw,
};

void rating_init(Rating* r, float x, float y, float star_size,
                 float value, u32 color, uint8_t editable,
                 void (*on_change)(float, void*), void* user_data) {
    widget_init(&r->base, x, y, star_size * 5.0f, star_size);
    r->base.vtable = &rating_vtable;
    r->base.user_data = user_data;
    r->value = value;
    r->star_color = color;
    r->empty_color = C_MUTED;
    r->editable = editable;
    r->on_change = on_change;
}
