/**
 * Modal — full-bottom-screen 3DS-style dialog (confirm / input / info).
 *
 * Overlay drawn on top of the active scene. Touch the OK/Cancel buttons to
 * close. For MODAL_INPUT, tapping the input field reopens the SWKBD.
 * modal_draw_top() dims the upper screen so background scenes look locked.
 * Modal is not a Widget; the scene calls modal_handle_touch / modal_draw_*
 * directly while modal.visible is set.
 */
#ifndef UI_MODAL_H
#define UI_MODAL_H

#include "widget.h"

typedef enum { MODAL_NONE, MODAL_CONFIRM, MODAL_INPUT, MODAL_INFO } ModalType;

typedef struct {
    ModalType type;
    uint8_t visible;
    char title[64];
    char body[256];
    char input_buf[128];     /* MODAL_INPUT payload */
    u32 bg_color;
    u32 overlay_color;       /* translucent dim layer */
    void (*on_confirm)(int ok, const char* input_text, void* user_data);
    void* cb_user_data;
} Modal;

void modal_init(Modal* m);
void modal_show_confirm(Modal* m, const char* title, const char* body,
                        void (*on_confirm)(int, const char*, void*), void* user_data);
void modal_show_info(Modal* m, const char* title, const char* body,
                     void (*on_confirm)(int, const char*, void*), void* user_data);
void modal_show_input(Modal* m, const char* title, const char* hint,
                      void (*on_confirm)(int, const char*, void*), void* user_data);
void modal_hide(Modal* m);
int  modal_handle_touch(Modal* m, const TouchState* t);
void modal_draw_bottom(Modal* m);
void modal_draw_top(Modal* m);

#endif /* UI_MODAL_H */
