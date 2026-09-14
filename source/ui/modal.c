/* Modal — full-bottom-screen 3DS-style dialog implementation. */
#include "modal.h"
#include "../core/utils.h"
#include <string.h>

#define MODAL_FRAME_W 240.0f
#define MODAL_FRAME_H 160.0f
#define MODAL_BTN_W   80.0f
#define MODAL_BTN_H   28.0f

static float modal_frame_x(void) { return (BOTTOM_W - MODAL_FRAME_W) * 0.5f; }
static float modal_frame_y(void) { return (BOTTOM_H - MODAL_FRAME_H) * 0.5f; }
static float modal_btn_y(void)   { return modal_frame_y() + MODAL_FRAME_H - MODAL_BTN_H - 10.0f; }
static float modal_ok_x(void)    { return modal_frame_x() + MODAL_FRAME_W - MODAL_BTN_W - 12.0f; }
static float modal_cancel_x(void){ return modal_frame_x() + 12.0f; }

static void modal_open(Modal* m, ModalType type, const char* title, const char* body,
                       void (*on_confirm)(int, const char*, void*), void* user_data) {
    m->type = type;
    m->visible = 1;
    utils_strlcpy(m->title, title ? title : "", sizeof(m->title));
    utils_strlcpy(m->body, body ? body : "", sizeof(m->body));
    m->input_buf[0] = '\0';
    m->on_confirm = on_confirm;
    m->cb_user_data = user_data;
}

void modal_init(Modal* m) {
    memset(m, 0, sizeof(*m));
    m->type = MODAL_NONE;
    m->visible = 0;
    m->bg_color = C_SURFACE;
    m->overlay_color = rgba8(0x00, 0x00, 0x00, 0xB0);
    m->title[0] = '\0';
    m->body[0] = '\0';
    m->input_buf[0] = '\0';
    m->on_confirm = NULL;
    m->cb_user_data = NULL;
}

void modal_show_confirm(Modal* m, const char* title, const char* body,
                        void (*on_confirm)(int, const char*, void*), void* user_data) {
    modal_open(m, MODAL_CONFIRM, title, body, on_confirm, user_data);
}

void modal_show_info(Modal* m, const char* title, const char* body,
                     void (*on_confirm)(int, const char*, void*), void* user_data) {
    modal_open(m, MODAL_INFO, title, body, on_confirm, user_data);
}

void modal_show_input(Modal* m, const char* title, const char* hint,
                      void (*on_confirm)(int, const char*, void*), void* user_data) {
    modal_open(m, MODAL_INPUT, title, hint, on_confirm, user_data);
}

void modal_hide(Modal* m) {
    m->visible = 0;
    m->type = MODAL_NONE;
}

int modal_handle_touch(Modal* m, const TouchState* t) {
    if (!m->visible) return 0;
    if (!t->touch_pressed) return 1;

    if (m->type == MODAL_INPUT) {
        float ix = modal_frame_x() + 12.0f;
        float iy = modal_frame_y() + MODAL_FRAME_H * 0.55f;
        float iw = MODAL_FRAME_W - 24.0f;
        float ih = 24.0f;
        if (t->tx >= ix && t->tx <= ix + iw && t->ty >= iy && t->ty <= iy + ih) {
            static SwkbdState swkbd;
            static char buf[128];
            swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 127);
            swkbdSetHintText(&swkbd, m->body[0] ? m->body : "Enter text...");
            swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
            swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK", true);
            if (swkbdInputText(&swkbd, buf, sizeof(buf)) == SWKBD_BUTTON_CONFIRM) {
                utils_strlcpy(m->input_buf, buf, sizeof(m->input_buf));
            }
            return 1;
        }
    }

    float by = modal_btn_y();
    if (t->tx >= modal_ok_x() && t->tx <= modal_ok_x() + MODAL_BTN_W &&
        t->ty >= by && t->ty <= by + MODAL_BTN_H) {
        if (m->on_confirm) m->on_confirm(1, m->input_buf, m->cb_user_data);
        modal_hide(m);
        return 1;
    }
    if (t->tx >= modal_cancel_x() && t->tx <= modal_cancel_x() + MODAL_BTN_W &&
        t->ty >= by && t->ty <= by + MODAL_BTN_H) {
        if (m->on_confirm) m->on_confirm(0, NULL, m->cb_user_data);
        modal_hide(m);
        return 1;
    }
    return 1;
}

void modal_draw_bottom(Modal* m) {
    if (!m->visible) return;
    render_draw_rect(0, 0, BOTTOM_W, BOTTOM_H, m->overlay_color);

    float fx = modal_frame_x(), fy = modal_frame_y();
    render_draw_rounded_rect(fx, fy, MODAL_FRAME_W, MODAL_FRAME_H, 6.0f, m->bg_color);
    render_draw_rect_outline(fx, fy, MODAL_FRAME_W, MODAL_FRAME_H, 2.0f, C_BLUE);

    if (m->title[0]) {
        render_draw_text(fx + 12.0f, fy + 10.0f, 0.7f, 0.7f, C_TEXT, "%s", m->title);
        render_draw_rect(fx + 12.0f, fy + 30.0f, MODAL_FRAME_W - 24.0f, 1.0f, C_MUTED);
    }
    if (m->body[0]) {
        render_draw_text(fx + 12.0f, fy + 40.0f, 0.5f, 0.5f, C_MUTED, "%s", m->body);
    }

    if (m->type == MODAL_INPUT) {
        float ix = fx + 12.0f;
        float iy = fy + MODAL_FRAME_H * 0.55f;
        float iw = MODAL_FRAME_W - 24.0f;
        float ih = 24.0f;
        render_draw_rounded_rect(ix, iy, iw, ih, 4.0f, C_BG);
        render_draw_rect_outline(ix, iy, iw, ih, 1.0f, C_MUTED);
        const char* disp = m->input_buf[0] ? m->input_buf
                          : (m->body[0] ? m->body : "");
        u32 col = m->input_buf[0] ? C_TEXT : C_MUTED;
        if (disp[0]) {
            render_draw_text(ix + 6.0f, iy + (ih - 8.0f * 0.6f) * 0.5f,
                             0.6f, 0.6f, col, "%s", disp);
        }
    }

    float by = modal_btn_y();
    float cx = modal_cancel_x();
    float ox = modal_ok_x();
    render_draw_rounded_rect(cx, by, MODAL_BTN_W, MODAL_BTN_H, 4.0f, C_BG);
    render_draw_text(cx + (MODAL_BTN_W - render_text_width(0.6f, "Cancel")) * 0.5f,
                     by + (MODAL_BTN_H - 8.0f * 0.6f) * 0.5f,
                     0.6f, 0.6f, C_TEXT, "Cancel");
    render_draw_rounded_rect(ox, by, MODAL_BTN_W, MODAL_BTN_H, 4.0f, C_BLUE);
    render_draw_text(ox + (MODAL_BTN_W - render_text_width(0.6f, "OK")) * 0.5f,
                     by + (MODAL_BTN_H - 8.0f * 0.6f) * 0.5f,
                     0.6f, 0.6f, C_SURFACE, "OK");
}

void modal_draw_top(Modal* m) {
    if (!m->visible) return;
    render_draw_rect(0, 0, TOP_W, TOP_H, m->overlay_color);
}
