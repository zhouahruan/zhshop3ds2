#include "transition.h"
#include "utils.h"

#include <3ds.h>
#include <math.h>

typedef enum { TR_IDLE, TR_IN, TR_OUT, TR_DONE } TrPhase;

static TrPhase s_phase = TR_IDLE;
static uint64_t s_start_ms = 0;
static const uint32_t DURATION_MS = 220;

void transition_init(void) {
    s_phase = TR_IDLE;
    s_start_ms = 0;
}

void transition_start_in(void) {
    s_phase = TR_IN;
    s_start_ms = osGetTime();
}

void transition_start_out(void) {
    s_phase = TR_OUT;
    s_start_ms = osGetTime();
}

int transition_in_progress(void) {
    return s_phase == TR_IN || s_phase == TR_OUT;
}

int transition_done(void) {
    return s_phase == TR_DONE;
}

static float tr_progress(void) {
    if (s_phase == TR_IDLE || s_phase == TR_DONE) return 0.0f;
    uint64_t now = osGetTime();
    float p = (float)(now - s_start_ms) / (float)DURATION_MS;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    return p;
}

void transition_update(void) {
    if (s_phase == TR_IDLE || s_phase == TR_DONE) return;
    float p = tr_progress();
    if (p >= 1.0f) {
        s_phase = TR_DONE;
    }
}

void transition_draw(ScreenId screen) {
    /* Dim overlay during transition fade. */
    float p = tr_progress();
    if (p <= 0.0f) return;
    float dim = 0.0f;
    if (s_phase == TR_IN) {
        /* Fade in to invisible: dim 0.5 -> 0. */
        dim = 0.5f * (1.0f - p);
    } else if (s_phase == TR_OUT) {
        /* Fade out: dim 0 -> 0.5. */
        dim = 0.5f * p;
    }
    if (dim <= 0.0f) return;

    u8 a = (u8)(dim * 255.0f);
    u32 color = rgba8(0x00, 0x00, 0x00, a);

    float w = (screen == SCREEN_TOP) ? (float)TOP_W : (float)BOTTOM_W;
    float h = (screen == SCREEN_TOP) ? (float)TOP_H : (float)BOTTOM_H;
    render_draw_rect(0, 0, w, h, color);
}

float transition_offset_x(ScreenId screen) {
    /* Not currently used; slide would offset scene draws.
     * Kept here so callers that want slide can pull the value. */
    (void)screen;
    return 0.0f;
}

float transition_alpha(void) {
    if (s_phase == TR_IN)  return tr_progress();
    if (s_phase == TR_OUT) return 1.0f - tr_progress();
    return 1.0f;
}
