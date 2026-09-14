/* Grid — paginated icon+label grid implementation. */
#include "grid.h"
#include "../core/utils.h"
#include <string.h>

static void grid_on_draw(Widget* self) {
    Grid* g = (Grid*)self;
    if (g->item_count <= 0 || !g->get_label) return;
    int per_page = g->cols * g->rows;
    for (int i = 0; i < per_page; i++) {
        int idx = g->offset + i;
        if (idx >= g->item_count) break;
        int r = i / g->cols;
        int c = i % g->cols;
        float x = g->base.x + (float)c * (g->item_w + g->gap);
        float y = g->base.y + (float)r * (g->item_h + g->gap);
        u32 bg = (idx == g->active) ? g->item_active_bg : g->item_bg;
        render_draw_rounded_rect(x, y, g->item_w, g->item_h, 4.0f, bg);
        const char** slot = g->get_label(idx, g->cb_user_data);
        const char* label = slot ? *slot : NULL;
        if (label) {
            float sx = 0.5f, sy = 0.5f;
            float tw = render_text_width(sx, label);
            float tx = x + (g->item_w - tw) * 0.5f;
            float ty = y + (g->item_h - 8.0f * sy) * 0.5f;
            render_draw_text(tx, ty, sx, sy, C_TEXT, "%s", label);
        }
    }
}

static int grid_on_touch(Widget* self, const TouchState* ts) {
    Grid* g = (Grid*)self;
    if (!g->base.enabled || g->item_count <= 0) return 0;
    if (!ts->touch_pressed || !widget_contains(&g->base, ts->tx, ts->ty)) return 0;
    int per_page = g->cols * g->rows;
    for (int i = 0; i < per_page; i++) {
        int idx = g->offset + i;
        if (idx >= g->item_count) break;
        int r = i / g->cols;
        int c = i % g->cols;
        float x = g->base.x + (float)c * (g->item_w + g->gap);
        float y = g->base.y + (float)r * (g->item_h + g->gap);
        if (ts->tx >= x && ts->tx <= x + g->item_w &&
            ts->ty >= y && ts->ty <= y + g->item_h) {
            g->active = idx;
            if (g->on_select) g->on_select(idx, g->cb_user_data);
            return 1;
        }
    }
    return 0;
}

static const WidgetVTable grid_vtable = {
    .on_touch = grid_on_touch,
    .on_draw = grid_on_draw,
};

void grid_init(Grid* g, float x, float y, float w, float h, int cols, int rows,
               float gap, void (*on_select)(int, void*), void* user_data) {
    widget_init(&g->base, x, y, w, h);
    g->base.vtable = &grid_vtable;
    g->cols = cols;
    g->rows = rows;
    g->gap = gap;
    g->item_count = 0;
    g->offset = 0;
    if (cols > 0) g->item_w = (w - (float)(cols - 1) * gap) / (float)cols;
    else g->item_w = w;
    if (rows > 0) g->item_h = (h - (float)(rows - 1) * gap) / (float)rows;
    else g->item_h = h;
    g->item_bg = C_SURFACE;
    g->item_active_bg = C_BLUE;
    g->active = -1;
    g->get_label = NULL;
    g->cb_user_data = user_data;
    g->on_select = on_select;
}

void grid_set_data(Grid* g, int count,
                   const char** (*get_label)(int, void*), void* cb_data) {
    g->item_count = count;
    g->get_label = get_label;
    g->cb_user_data = cb_data;
    if (g->offset < 0) g->offset = 0;
    int per_page = g->cols * g->rows;
    int max_off = count - per_page;
    if (max_off < 0) max_off = 0;
    if (g->offset > max_off) g->offset = max_off;
}

void grid_scroll(Grid* g, int delta) {
    int per_page = g->cols * g->rows;
    int max_off = g->item_count - per_page;
    if (max_off < 0) max_off = 0;
    g->offset += delta;
    if (g->offset < 0) g->offset = 0;
    if (g->offset > max_off) g->offset = max_off;
}
