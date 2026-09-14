/* Tabs — horizontal tab bar implementation. */
#include "tabs.h"

static void tabs_on_draw(Widget* self) {
    Tabs* t = (Tabs*)self;
    if (t->count <= 0) return;
    float tab_w = t->base.w / (float)t->count;
    float sx = 0.5f, sy = 0.5f;
    for (int i = 0; i < t->count; i++) {
        float tx = t->base.x + (float)i * tab_w;
        u32 col = (i == t->active) ? t->active_color : t->inactive_color;
        render_draw_rect(tx, t->base.y, tab_w, t->base.h, col);
        if (i == t->active) {
            render_draw_rect(tx, t->base.y + t->base.h - 2.0f, tab_w, 2.0f,
                             t->active_color);
        }
        if (t->labels && t->labels[i]) {
            float tw = render_text_width(sx, t->labels[i]);
            render_draw_text(tx + (tab_w - tw) * 0.5f,
                             t->base.y + (t->base.h - 8.0f * sy) * 0.5f,
                             sx, sy, t->text_color, "%s", t->labels[i]);
        }
    }
}

static int tabs_on_touch(Widget* self, const TouchState* ts) {
    Tabs* t = (Tabs*)self;
    if (!t->base.enabled || t->count <= 0) return 0;
    if (ts->touch_pressed && widget_contains(&t->base, ts->tx, ts->ty)) {
        float tab_w = t->base.w / (float)t->count;
        int idx = (int)((ts->tx - t->base.x) / tab_w);
        if (idx < 0) idx = 0;
        if (idx >= t->count) idx = t->count - 1;
        if (idx != t->active) {
            t->active = idx;
            if (t->on_change) t->on_change(idx, t->base.user_data);
        }
        return 1;
    }
    return 0;
}

static const WidgetVTable tabs_vtable = {
    .on_touch = tabs_on_touch,
    .on_draw = tabs_on_draw,
};

void tabs_init(Tabs* t, float x, float y, float w, float h,
              const char** labels, int count, int initial,
              u32 active_color, u32 inactive_color, u32 text_color,
              void (*on_change)(int, void*), void* user_data) {
    widget_init(&t->base, x, y, w, h);
    t->base.vtable = &tabs_vtable;
    t->base.user_data = user_data;
    t->labels = labels;
    t->count = count;
    t->active = initial;
    t->active_color = active_color;
    t->inactive_color = inactive_color;
    t->text_color = text_color;
    t->on_change = on_change;
}
