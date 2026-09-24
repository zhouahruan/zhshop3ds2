/* SearchBar — text input with SWKBD implementation. */
#include "searchbar.h"
#include "../core/utils.h"
#include <string.h>

static void searchbar_on_draw(Widget* self) {
    SearchBar* b = (SearchBar*)self;
    render_draw_rounded_rect(b->base.x, b->base.y, b->base.w, b->base.h,
                             4.0f, b->bg_color);
    render_draw_rect_outline(b->base.x, b->base.y, b->base.w, b->base.h,
                             1.0f, b->border_color);
    const char* display = b->text[0] ? b->text : "Search...";
    u32 col = b->text[0] ? b->text_color : C_MUTED;
    float sy = 0.6f;
    float ty = b->base.y + (b->base.h - 8.0f * sy) * 0.5f;
    render_draw_text(b->base.x + 8.0f, ty, 0.6f, sy, col, "%s", display);
}

static int searchbar_on_touch(Widget* self, const TouchState* ts) {
    SearchBar* b = (SearchBar*)self;
    if (!b->base.enabled) return 0;
    if (!ts->touch_pressed || !widget_contains(&b->base, ts->tx, ts->ty)) return 0;
    static SwkbdState swkbd;
    static char buf[128];
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
    swkbdSetHintText(&swkbd, "Search apps...");
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK", true);
    if (swkbdInputText(&swkbd, buf, sizeof(buf)) == SWKBD_BUTTON_CONFIRM) {
        utils_strlcpy(b->text, buf, sizeof(b->text));
        if (b->on_submit) b->on_submit(b->text, b->cb_user_data);
    }
    return 1;
}

static const WidgetVTable searchbar_vtable = {
    .on_touch = searchbar_on_touch,
    .on_draw = searchbar_on_draw,
};

void searchbar_init(SearchBar* b, float x, float y, float w, float h,
                    void (*on_submit)(const char*, void*), void* user_data) {
    widget_init(&b->base, x, y, w, h);
    b->base.vtable = &searchbar_vtable;
    b->text[0] = '\0';
    b->bg_color = C_SURFACE;
    b->border_color = C_MUTED;
    b->text_color = C_TEXT;
    b->on_submit = on_submit;
    b->cb_user_data = user_data;
}

const char* searchbar_text(const SearchBar* b) {
    return b->text;
}
