/**
 * Carousel — banner image auto-rotator.
 *
 * Shows one slide from an image array at a time and switches on a timer.
 * Page dots indicate the active slide. Touch the left/right half to step.
 * Because Widget's vtable has no update hook, the scene must call
 * carousel_update() each frame.
 */
#ifndef UI_CAROUSEL_H
#define UI_CAROUSEL_H

#include "widget.h"

typedef struct {
    Widget base;
    C2D_Image* images;     /* image array (externally owned) */
    int count;
    int current;
    uint64_t last_switch_ms;
    uint32_t interval_ms;   /* default 3000 */
    uint8_t auto_play;
} Carousel;

void carousel_init(Carousel* c, float x, float y, float w, float h);
void carousel_set_images(Carousel* c, C2D_Image* imgs, int count);
void carousel_update(Carousel* c, uint64_t now_ms);

#endif /* UI_CAROUSEL_H */
