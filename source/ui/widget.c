#include "widget.h"

#include <string.h>

void widget_init(Widget* w, float x, float y, float w_, float h_) {
    if (!w) return;
    memset(w, 0, sizeof(*w));
    w->x = x; w->y = y; w->w = w_; w->h = h_;
    w->visible = 1;
    w->enabled = 1;
}

int widget_contains(const Widget* w, float px, float py) {
    if (!w) return 0;
    return px >= w->x && px <= w->x + w->w &&
           py >= w->y && py <= w->y + w->h;
}

int widget_touch(Widget* w, const TouchState* t) {
    if (!w || !w->visible || !w->enabled || !w->vtable || !w->vtable->on_touch)
        return 0;
    return w->vtable->on_touch(w, t);
}

void widget_draw(Widget* w) {
    if (!w || !w->visible || !w->vtable || !w->vtable->on_draw) return;
    w->vtable->on_draw(w);
}
