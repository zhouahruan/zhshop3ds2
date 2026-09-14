#include "app.h"

#include <string.h>

AppState g_app;

void app_init(void) {
    memset(&g_app, 0, sizeof(g_app));
    g_app.last_fps_time = osGetTime();
}

void app_exit(void) {
    /* Nothing dynamically allocated; safe no-op. */
}

void app_request_quit(void) {
    g_app.quit_requested = 1;
}
