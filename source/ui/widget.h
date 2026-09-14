/**
 * Widget base type — common fields + virtual draw/touch hooks.
 *
 * Most UI components embed a Widget struct as their first member so a
 * pointer to a Button, Label, etc. can be cast to Widget* and treated
 * generically. Touch info comes from core/input.
 */
#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include "../core/render.h"
#include "../core/input.h"
#include <stdint.h>

typedef struct Widget Widget;

typedef struct {
    /* Return 1 if event consumed. */
    int (*on_touch)(Widget* self, const TouchState* t);
    void (*on_draw)(Widget* self);
} WidgetVTable;

struct Widget {
    float x, y, w, h;
    uint8_t visible;
    uint8_t enabled;
    uint8_t pressed;          /* transient pressed-down highlight */
    const WidgetVTable* vtable;
    void* user_data;          /* back-pointer to the embedding component */
};

/* Init helper for first-time setup. */
void widget_init(Widget* w, float x, float y, float w_, float h_);

/* True if (px,py) falls inside w's bounds. */
int  widget_contains(const Widget* w, float px, float py);

/* Generic dispatcher that callers should route through. */
int  widget_touch(Widget* w, const TouchState* t);
void widget_draw(Widget* w);

#endif /* UI_WIDGET_H */
