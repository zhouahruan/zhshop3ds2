/**
 * Scene transition animation (3D flip / fade).
 *
 * A push-out animation runs first (current scene rotates away), then
 * a push-in animation rotates the new scene in. To keep this simple and
 * working with the citro2d 2D drawing pipeline, transitions are
 * implemented as a slide + fade composite using rect tinted overlays
 * and an x-offset applied to draws.
 *
 * If you want true 3D rotateY later, the entry point is transition_draw
 * which can wrap each scene draw in a matrix push/pop.
 */
#ifndef CORE_TRANSITION_H
#define CORE_TRANSITION_H

#include "render.h"

void transition_init(void);

/* Kick off a forward (in) transition. */
void transition_start_in(void);
/* Kick off a backward (out) transition. */
void transition_start_out(void);

/* True while a transition is running. */
int  transition_in_progress(void);

/* True when transition has finished and pending navigation can apply. */
int  transition_done(void);

/* Per-frame tick. Updates internal phase timer. */
void transition_update(void);

/* Optional overlay drawing: faded bar across the screen during transitions. */
void transition_draw(ScreenId screen);

/* Horizontal offset (in pixels) to apply to current scene draws.
 * 0 when no transition. Positive shifts content right (slide-in from left). */
float transition_offset_x(ScreenId screen);

/* Alpha 0..1 to multiply onto scene draws (1.0 = fully visible). */
float transition_alpha(void);

#endif /* CORE_TRANSITION_H */
