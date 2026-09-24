/**
 * Button — 3D-style raised push button with click callback.
 */
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "widget.h"

typedef struct {
    Widget base;
    const char* label;
    u32 bg_color;
    u32 text_color;
    void (*on_click)(void* user_data);
} Button;

void button_init(Button* b, float x, float y, float w, float h,
                 const char* label, u32 bg, u32 text_color,
                 void (*on_click)(void*), void* user_data);

#endif /* UI_BUTTON_H */
