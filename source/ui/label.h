/**
 * Label — text label with simple character-based word wrap.
 */
#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "widget.h"

typedef struct {
    Widget base;
    const char* text;
    u32 color;
    float scale_x, scale_y;
    uint8_t wrap;
} Label;

void label_init(Label* l, float x, float y, float w, float h,
               const char* text, u32 color, float scale);
void label_set_text(Label* l, const char* text);

#endif /* UI_LABEL_H */
