/**
 * Tabs — horizontal tab bar with touch-based selection.
 */
#ifndef UI_TABS_H
#define UI_TABS_H

#include "widget.h"

typedef struct {
    Widget base;
    const char** labels;
    int count;
    int active;
    u32 active_color;
    u32 inactive_color;
    u32 text_color;
    void (*on_change)(int new_index, void* user_data);
} Tabs;

void tabs_init(Tabs* t, float x, float y, float w, float h,
              const char** labels, int count, int initial,
              u32 active_color, u32 inactive_color, u32 text_color,
              void (*on_change)(int, void*), void* user_data);

#endif /* UI_TABS_H */
