/* List — vertically scrolling list implementation. */
#include "list.h"
#include "../core/utils.h"
#include <string.h>

static float list_content_w(const List* l) {
    return l->base.w - l->scrollbar.base.w - 4.0f;
}

static void list_on_draw(Widget* self) {
    List* l = (List*)self;
    float lw = list_content_w(l);
    if (l->item_count > 0 && l->get_label) {
        for (int i = 0; i < l->visible_items; i++) {
            int idx = l->offset + i;
            if (idx >= l->item_count) break;
            float x = l->base.x, y = l->base.y + (float)i * l->item_h;
            u32 bg = (idx == l->active) ? l->item_active_bg : l->item_bg;
            render_draw_rounded_rect(x, y, lw, l->item_h, 4.0f, bg);
            const char* label = l->get_label(idx, l->cb_user_data);
            if (label) {
                float sy = 0.6f;
                float ty = y + (l->item_h - 8.0f * sy) * 0.5f;
                render_draw_text(x + 8.0f, ty, 0.6f, sy, C_TEXT, "%s", label);
            }
        }
    }
    scrollbar_set(&l->scrollbar, l->item_count, l->visible_items, l->offset);
    widget_draw(&l->scrollbar.base);
}

static int list_on_touch(Widget* self, const TouchState* ts) {
    List* l = (List*)self;
    if (!l->base.enabled || l->item_count <= 0) return 0;
    if (!ts->touch_released || !widget_contains(&l->base, ts->tx, ts->ty)) return 0;
    float lw = list_content_w(l);
    if (ts->tx > l->base.x + lw) return 1;
    int row = (int)((ts->ty - l->base.y) / l->item_h);
    if (row < 0) row = 0;
    if (row >= l->visible_items) row = l->visible_items - 1;
    int idx = l->offset + row;
    if (idx >= l->item_count) idx = l->item_count - 1;
    if (idx >= 0) {
        l->active = idx;
        if (l->on_select) l->on_select(idx, l->cb_user_data);
    }
    return 1;
}

static const WidgetVTable list_vtable = {
    .on_touch = list_on_touch,
    .on_draw = list_on_draw,
};

void list_init(List* l, float x, float y, float w, float h, float item_h,
               void (*on_select)(int, void*), void* user_data) {
    widget_init(&l->base, x, y, w, h);
    l->base.vtable = &list_vtable;
    l->item_count = 0;
    l->visible_items = (item_h > 0.0f) ? (int)(h / item_h) : 0;
    l->offset = 0;
    l->item_h = item_h;
    l->item_bg = C_SURFACE;
    l->item_active_bg = C_BLUE;
    l->active = -1;
    l->get_label = NULL;
    l->cb_user_data = user_data;
    l->on_select = on_select;
    scrollbar_init(&l->scrollbar, x + w - 8.0f, y, h, C_BLUE, C_MUTED);
}

void list_set_data(List* l, int count,
                   const char* (*get_label)(int, void*), void* cb_data) {
    l->item_count = count;
    l->get_label = get_label;
    l->cb_user_data = cb_data;
    if (l->offset < 0) l->offset = 0;
    int max_off = count - l->visible_items;
    if (max_off < 0) max_off = 0;
    if (l->offset > max_off) l->offset = max_off;
}

void list_scroll(List* l, int delta) {
    int max_off = l->item_count - l->visible_items;
    if (max_off < 0) max_off = 0;
    l->offset += delta;
    if (l->offset < 0) l->offset = 0;
    if (l->offset > max_off) l->offset = max_off;
}
