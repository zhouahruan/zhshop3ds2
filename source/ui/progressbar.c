/* ProgressBar — progress bar implementation. */
#include "progressbar.h"
#include "../core/utils.h"
#include <stdio.h>

static void progressbar_on_draw(Widget* self) {
    ProgressBar* p = (ProgressBar*)self;
    float bx = p->base.x, by = p->base.y, bw = p->base.w, bh = p->base.h;
    render_draw_rounded_rect(bx, by, bw, bh, 2.0f, p->track_color);
    float frac = (float)p->progress / 100.0f;
    if (frac < 0.0f) frac = 0.0f;
    if (frac > 1.0f) frac = 1.0f;
    float bar_w = bw * frac;
    if (bar_w > 0.0f) {
        render_draw_rounded_rect(bx, by, bar_w, bh, 2.0f, p->bar_color);
    }
    float sx = 0.5f, sy = 0.5f;
    char pct[16];
    snprintf(pct, sizeof(pct), "%d%%", p->progress);
    float tw = render_text_width(sx, pct);
    float tx = bx + (bw - tw) * 0.5f;
    float ty = by + (bh - 8.0f * sy) * 0.5f;
    render_draw_text(tx, ty, sx, sy, p->text_color, "%s", pct);
    if (p->label) {
        float lw = render_text_width(sx, p->label);
        render_draw_text(bx + (bw - lw) * 0.5f, by + bh + 2.0f,
                         sx, sy, p->text_color, "%s", p->label);
    }
}

static const WidgetVTable progressbar_vtable = {
    .on_touch = NULL,
    .on_draw = progressbar_on_draw,
};

void progressbar_init(ProgressBar* p, float x, float y, float w, float h,
                      u32 bar_color, u32 text_color) {
    widget_init(&p->base, x, y, w, h);
    p->base.vtable = &progressbar_vtable;
    p->progress = 0;
    p->bar_color = bar_color;
    p->track_color = C_MUTED;
    p->text_color = text_color;
    p->label = NULL;
}

void progressbar_set(ProgressBar* p, int progress, const char* label) {
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    p->progress = progress;
    p->label = label;
}
