/**
 * Application-level global state.
 * Holds flags shared by multiple subsystems (quit request, debug toggles).
 * Does NOT hold data domain state — that lives in data/store.
 */
#ifndef CORE_APP_H
#define CORE_APP_H

#include <3ds.h>
#include <stdint.h>

typedef struct {
    uint8_t quit_requested;
    uint8_t debug_overlay;
    uint8_t show_fps;
    /* Stats updated by main loop. */
    uint32_t frame_count;
    uint32_t last_fps;
    uint64_t last_fps_time;
} AppState;

extern AppState g_app;

void app_init(void);
void app_exit(void);

/* Mark the app to exit at end of current frame. */
void app_request_quit(void);

#endif /* CORE_APP_H */
