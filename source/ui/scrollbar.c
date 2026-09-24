/* ScrollBar — vertical scroll indicator implementation. */
#include "scrollbar.h"

static void scrollbar_on_draw(Widget* self) {
    ScrollBar* s = (ScrollBar*)self;
    float x = s->base.x, y = s->base.y, w = s->base.w, h = s->base.h;
    render_draw_rect(x, y, w, h, s->track_color);
    if (s->total_items <= 0 || s->visible_items <= 0) return;
    float ratio = (float)s->visible_items / (float)s->total_items;
    if (ratio > 1.0f) ratio = 1.0f;
    float thumb_h = h * ratio;
    if (thumb_h < 4.0f) thumb_h = 4.0f;
    float off_ratio = (float)s->offset / (float)s->total_items;
    if (off_ratio < 0.0f) off_ratio = 0.0f;
    if (off_ratio > 1.0f) off_ratio = 1.0f;
    float thumb_y = y + h * off_ratio;
    if (thumb_y + thumb_h > y + h) thumb_y = y + h - thumb_h;
    render_draw_rect(x, thumb_y, w, thumb_h, s->thumb_color);
}

static const WidgetVTable scrollbar_vtable = {
    .on_touch = NULL,
    .on_draw = scrollbar_on_draw,
};

void scrollbar_init(ScrollBar* s, float x, float y, float h,
                    u32 thumb, u32 track) {
    widget_init(&s->base, x, y, 4.0f, h);
    s->base.vtable = &scrollbar_vtable;
    s->total_items = 0;
    s->visible_items = 0;
    s->offset = 0;
    s->track_color = track;
    s->thumb_color = thumb;
}

void scrollbar_set(ScrollBar* s, int total, int visible, int offset) {
    s->total_items = total;
    s->visible_items = visible;
    s->offset = offset;
}
