/**
 * ScrollBar — vertical scroll indicator (display only, no drag).
 */
#ifndef UI_SCROLLBAR_H
#define UI_SCROLLBAR_H

#include "widget.h"

typedef struct {
    Widget base;
    int total_items;
    int visible_items;
    int offset;
    u32 track_color;
    u32 thumb_color;
} ScrollBar;

void scrollbar_init(ScrollBar* s, float x, float y, float h,
                    u32 thumb, u32 track);
void scrollbar_set(ScrollBar* s, int total, int visible, int offset);

#endif /* UI_SCROLLBAR_H */
