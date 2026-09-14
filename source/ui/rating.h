/**
 * Rating — 0..5 star rating with half-star precision.
 */
#ifndef UI_RATING_H
#define UI_RATING_H

#include "widget.h"

typedef struct {
    Widget base;
    float value;
    u32 star_color;
    u32 empty_color;
    uint8_t editable;
    void (*on_change)(float new_value, void* user_data);
} Rating;

void rating_init(Rating* r, float x, float y, float star_size,
                 float value, u32 color, uint8_t editable,
                 void (*on_change)(float, void*), void* user_data);

#endif /* UI_RATING_H */
