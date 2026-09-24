/* Label — text label implementation with optional wrap. */
#include "label.h"
#include "../core/utils.h"
#include <string.h>

static void label_on_draw(Widget* self) {
    Label* l = (Label*)self;
    if (!l->text) return;
    if (!l->wrap) {
        render_draw_text(l->base.x, l->base.y, l->scale_x, l->scale_y,
                         l->color, "%s", l->text);
        return;
    }
    float char_w = 8.0f * l->scale_x;
    int max_chars = (int)(l->base.w / char_w);
    if (max_chars < 1) max_chars = 1;
    float line_h = 10.0f * l->scale_y;
    float y = l->base.y;
    int len = (int)strlen(l->text);
    int pos = 0;
    while (pos < len) {
        int chunk = max_chars;
        if (chunk > 255) chunk = 255;
        if (pos + chunk > len) chunk = len - pos;
        char buf[256];
        utils_strlcpy(buf, l->text + pos, (size_t)chunk + 1);
        render_draw_text(l->base.x, y, l->scale_x, l->scale_y,
                         l->color, "%s", buf);
        y += line_h;
        pos += chunk;
    }
}

static const WidgetVTable label_vtable = {
    .on_touch = NULL,
    .on_draw = label_on_draw,
};

void label_init(Label* l, float x, float y, float w, float h,
               const char* text, u32 color, float scale) {
    widget_init(&l->base, x, y, w, h);
    l->base.vtable = &label_vtable;
    l->text = text;
    l->color = color;
    l->scale_x = scale;
    l->scale_y = scale;
    l->wrap = 0;
}

void label_set_text(Label* l, const char* text) {
    l->text = text;
}
