/**
 * Grid — paginated icon+label grid with touch selection.
 *
 * Items are addressed by index. The grid draws cols*rows visible items
 * starting at `offset`. Touching an item selects it and fires on_select.
 */
#ifndef UI_GRID_H
#define UI_GRID_H

#include "widget.h"

typedef struct {
    Widget base;
    int cols;              /* column count (item_w = w/cols) */
    int rows;              /* visible row count */
    int item_count;        /* total items */
    int offset;            /* current scroll offset (in items) */
    float item_w, item_h;  /* per-cell size */
    float gap;             /* gap between cells */
    u32 item_bg;
    u32 item_active_bg;
    int active;            /* selected index, -1 = none */
    const char** (*get_label)(int idx, void* user_data);
    void* cb_user_data;
    void (*on_select)(int idx, void* user_data);
} Grid;

void grid_init(Grid* g, float x, float y, float w, float h, int cols, int rows,
               float gap, void (*on_select)(int, void*), void* user_data);
void grid_set_data(Grid* g, int count,
                   const char** (*get_label)(int, void*), void* cb_data);
void grid_scroll(Grid* g, int delta);

#endif /* UI_GRID_H */
