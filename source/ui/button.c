/* Button — 3D-style raised push button implementation. */
#include "button.h"
#include "../core/utils.h"

static void button_on_draw(Widget* self) {
    Button* b = (Button*)self;
    float off = b->base.pressed ? 2.0f : 0.0f;
    float bx = b->base.x;
    float by = b->base.y + off;
    float bw = b->base.w;
    float bh = b->base.h;
    render_draw_rounded_rect(bx, by, bw, bh, 4.0f, b->bg_color);
    render_draw_rect(bx + 2.0f, by + 1.0f, bw - 4.0f, 1.0f, C_SURFACE);
    if (b->label) {
        float sx = 0.6f, sy = 0.6f;
        float tw = render_text_width(sx, b->label);
        float tx = bx + (bw - tw) * 0.5f;
        float ty = by + (bh - 8.0f * sy) * 0.5f;
        render_draw_text(tx, ty, sx, sy, b->text_color, "%s", b->label);
    }
}

static int button_on_touch(Widget* self, const TouchState* ts) {
    Button* b = (Button*)self;
    if (!b->base.enabled) return 0;
    if (ts->touch_pressed && widget_contains(&b->base, ts->tx, ts->ty)) {
        b->base.pressed = 1;
        if (b->on_click) b->on_click(b->base.user_data);
        return 1;
    }
    if (ts->touch_released && b->base.pressed) {
        b->base.pressed = 0;
        return 1;
    }
    return 0;
}

static const WidgetVTable button_vtable = {
    .on_touch = button_on_touch,
    .on_draw = button_on_draw,
};

void button_init(Button* b, float x, float y, float w, float h,
                 const char* label, u32 bg, u32 text_color,
                 void (*on_click)(void*), void* user_data) {
    widget_init(&b->base, x, y, w, h);
    b->base.vtable = &button_vtable;
    b->base.user_data = user_data;
    b->label = label;
    b->bg_color = bg;
    b->text_color = text_color;
    b->on_click = on_click;
}
