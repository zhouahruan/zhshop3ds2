/**
 * SearchBar — text input that opens the 3DS software keyboard (SWKBD).
 *
 * Touch the bar to launch swkbd. On confirm the text is stored and on_submit fires.
 * An empty field draws a grey placeholder.
 */
#ifndef UI_SEARCHBAR_H
#define UI_SEARCHBAR_H

#include "widget.h"

typedef struct {
    Widget base;
    char text[128];
    u32 bg_color;
    u32 border_color;
    u32 text_color;
    void (*on_submit)(const char* text, void* user_data);
    void* cb_user_data;
} SearchBar;

void searchbar_init(SearchBar* b, float x, float y, float w, float h,
                    void (*on_submit)(const char*, void*), void* user_data);
const char* searchbar_text(const SearchBar* b);

#endif /* UI_SEARCHBAR_H */
