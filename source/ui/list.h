/**
 * List — vertically scrolling list with embedded scrollbar.
 *
 * One row per item at fixed item_h height. Touching an item on release
 * selects it and fires on_select. The scrollbar is drawn at the right edge.
 */
#ifndef UI_LIST_H
#define UI_LIST_H

#include "widget.h"
#include "scrollbar.h"

typedef struct {
    Widget base;
    int item_count;
    int visible_items;
    int offset;
    float item_h;
    u32 item_bg;
    u32 item_active_bg;
    int active;            /* selected index, -1 = none */
    const char* (*get_label)(int idx, void* user_data);
    void* cb_user_data;
    void (*on_select)(int idx, void* user_data);
    ScrollBar scrollbar;
} List;

void list_init(List* l, float x, float y, float w, float h, float item_h,
               void (*on_select)(int, void*), void* user_data);
void list_set_data(List* l, int count,
                   const char* (*get_label)(int, void*), void* cb_data);
void list_scroll(List* l, int delta);

#endif /* UI_LIST_H */
