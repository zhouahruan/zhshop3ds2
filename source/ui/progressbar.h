/**
 * ProgressBar — download progress bar with percentage and optional label.
 */
#ifndef UI_PROGRESSBAR_H
#define UI_PROGRESSBAR_H

#include "widget.h"

typedef struct {
    Widget base;
    int progress;
    u32 bar_color;
    u32 track_color;
    u32 text_color;
    const char* label;
} ProgressBar;

void progressbar_init(ProgressBar* p, float x, float y, float w, float h,
                      u32 bar_color, u32 text_color);
void progressbar_set(ProgressBar* p, int progress, const char* label);

#endif /* UI_PROGRESSBAR_H */
