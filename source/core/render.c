#include "render.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static C3D_RenderTarget* s_top_left   = NULL;
static C3D_RenderTarget* s_top_right  = NULL;
static C3D_RenderTarget* s_bottom     = NULL;

static C2D_TextBuf s_text_buf   = NULL;
static C3D_RenderTarget* s_target = NULL;   /* currently-bound target */

void render_init(void) {
    s_top_left  = C2D_CreateScreenTarget(GFX_TOP,    GFX_LEFT);
    s_top_right = C2D_CreateScreenTarget(GFX_TOP,    GFX_RIGHT);
    s_bottom    = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    s_text_buf = C2D_TextBufNew(4096);
    s_target = NULL;
}

void render_exit(void) {
    if (s_text_buf) {
        C2D_TextBufClear(s_text_buf);
        C2D_TextBufDelete(s_text_buf);
        s_text_buf = NULL;
    }
    /* Render targets freed by C2D_Fini / C3D_Fini. */
    s_top_left = s_top_right = s_bottom = NULL;
}

void render_begin_frame(void) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    s_target = NULL;
    /* Reset text buffer each frame to avoid overflow. */
    if (s_text_buf) C2D_TextBufClear(s_text_buf);
}

void render_target_top(Eye eye) {
    if (eye == EYE_LEFT)        s_target = s_top_left;
    else if (eye == EYE_RIGHT) s_target = s_top_right;
    else                        s_target = s_top_left;  /* 2D mode: just use left buffer */
    if (!s_target) return;
    C2D_TargetClear(s_target, C_SCREEN_BG);
    C2D_SceneBegin(s_target);
}

void render_target_bottom(void) {
    s_target = s_bottom;
    if (!s_target) return;
    C2D_TargetClear(s_target, C_SCREEN_BG);
    C2D_SceneBegin(s_target);
}

void render_end_frame(void) {
    C2D_Flush();
    C3D_FrameEnd(0);
    s_target = NULL;
}

void render_clear(u32 color) {
    if (s_target) C2D_TargetClear(s_target, color);
}

void render_draw_rect(float x, float y, float w, float h, u32 color) {
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}

void render_draw_rect_outline(float x, float y, float w, float h, float line_w, u32 color) {
    C2D_DrawRectSolid(x,             y,             0.5f, w,      line_w, color);  /* top    */
    C2D_DrawRectSolid(x,      y + h - line_w,      0.5f, w,      line_w, color);  /* bottom */
    C2D_DrawRectSolid(x,             y,             0.5f, line_w, h,      color);  /* left   */
    C2D_DrawRectSolid(x + w - line_w, y,            0.5f, line_w, h,      color);  /* right  */
}

void render_draw_rounded_rect(float x, float y, float w, float h, float r, u32 color) {
    /* citro2d's DrawRectsolid doesn't have a radius primitive; approximate with
     * body + 4 corner circles. */
    C2D_DrawRectSolid(x + r, y,         0.5f, w - 2 * r, h,     color);
    C2D_DrawRectSolid(x,     y + r,     0.5f, 2 * r,      h - 2 * r, color);
    C2D_DrawCircleSolid(x + r,         y + r,         0.5f, r, color);
    C2D_DrawCircleSolid(x + w - r,     y + r,         0.5f, r, color);
    C2D_DrawCircleSolid(x + r,         y + h - r,     0.5f, r, color);
    C2D_DrawCircleSolid(x + w - r,     y + h - r,     0.5f, r, color);
}

void render_draw_text(float x, float y, float scale_x, float scale_y, u32 color,
                      const char* fmt, ...) {
    if (!s_text_buf) return;
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    C2D_Text text;
    C2D_TextParse(&text, s_text_buf, buf);
    C2D_TextOptimize(&text);

    C2D_DrawText(&text, C2D_WithColor, x, y, 0.5f, scale_x, scale_y, color);
}

float render_text_width(float scale_x, const char* str) {
    if (!s_text_buf) return 0.0f;
    C2D_Text t;
    C2D_TextParse(&t, s_text_buf, str);
    float w = 0.0f, h = 0.0f;
    C2D_TextGetDimensions(&t, scale_x, scale_x, &w, &h);
    return w;
}

void render_draw_sprite(C2D_Sprite* spr) {
    C2D_DrawSprite(spr);
}
