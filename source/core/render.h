/**
 * citro3d + citro2d rendering wrapper.
 *
 * 3DS dual-screen: top 400x240 (800x240 in 3D, two eyes 400x240 each),
 * bottom 320x240 (touch).
 */
#ifndef CORE_RENDER_H
#define CORE_RENDER_H

#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <stdint.h>
#include <stddef.h>

typedef enum { SCREEN_TOP, SCREEN_BOTTOM } ScreenId;
typedef enum { EYE_LEFT, EYE_RIGHT, EYE_2D } Eye;

/*---- Lifecycle ----*/
void render_init(void);
void render_exit(void);

/*---- Per-frame ----*/
void render_begin_frame(void);
void render_target_top(Eye eye);
void render_target_bottom(void);
void render_end_frame(void);

/*---- Drawing primitives ----*/
void render_clear(u32 color);

/* Solid rectangle. */
void render_draw_rect(float x, float y, float w, float h, u32 color);

/* Rectangle outline. */
void render_draw_rect_outline(float x, float y, float w, float h, float line_w, u32 color);

/* Rounded rectangle filled. */
void render_draw_rounded_rect(float x, float y, float w, float h, float r, u32 color);

/* Text. y is baseline top. scale 1.0 == 3ds default. */
void render_draw_text(float x, float y, float scale_x, float scale_y, u32 color,
                      const char* fmt, ...);

/* Measure text width at given scale_x. */
float render_text_width(float scale_x, const char* text);

/* Sprite draw helper. C2D_Sprite must already be set up. */
void render_draw_sprite(C2D_Sprite* spr);

/* Screen dimensions (constant). */
#define TOP_W       400
#define TOP_H       240
#define BOTTOM_W    320
#define BOTTOM_H    240

#endif /* CORE_RENDER_H */
