#include "input.h"
#include <string.h>

static uint32_t s_held      = 0;
static uint32_t s_pressed   = 0;
static uint32_t s_released  = 0;
static uint32_t s_prev_held = 0;

static TouchState s_touch;
static touchPosition s_cur_touch;
static uint64_t      s_last_tap_time = 0;
static uint8_t       s_last_tap_valid = 0;

/* Circle pad state. */
static circlePosition s_cpad;
static int s_cpad_x = 0;
static int s_cpad_y = 0;

/* Gamepad navigation: tracks held directions with repeat timing. */
#define CPAD_DEADZONE  25
#define CPAD_THRESHOLD 45  /* beyond deadzone threshold to trigger direction */

static uint8_t s_nav_pressed = 0;  /* edge this frame */
static uint8_t s_nav_held    = 0;  /* currently held */

/* Repeat timing: first press fires immediately, then repeats after
 * INITIAL_DELAY ms, repeating every REPEAT_INTERVAL ms. */
#define NAV_INITIAL_DELAY   400  /* ms before auto-repeat starts */
#define NAV_REPEAT_INTERVAL 120  /* ms between repeats */

typedef struct {
    uint8_t mask;         /* NAV_UP etc */
    uint64_t hold_start;  /* osGetTime when direction first held */
    uint64_t last_repeat;  /* last time we emitted a repeat */
} NavDirState;

static NavDirState s_nav_states[4];

void input_init(void) {
    memset(&s_touch, 0, sizeof(s_touch));
    memset(&s_nav_states, 0, sizeof(s_nav_states));
    s_nav_states[0].mask = NAV_UP;
    s_nav_states[1].mask = NAV_DOWN;
    s_nav_states[2].mask = NAV_LEFT;
    s_nav_states[3].mask = NAV_RIGHT;
    hidInit();
}

/* Compute held direction mask from D-pad + circle pad. */
static uint8_t compute_nav_held(void) {
    uint8_t mask = 0;
    /* D-pad. */
    if (s_held & KEY_DUP)    mask |= NAV_UP;
    if (s_held & KEY_DDOWN)  mask |= NAV_DOWN;
    if (s_held & KEY_DLEFT)  mask |= NAV_LEFT;
    if (s_held & KEY_DRIGHT) mask |= NAV_RIGHT;
    /* Circle pad. */
    if (s_cpad_y >=  CPAD_THRESHOLD) mask |= NAV_UP;
    if (s_cpad_y <= -CPAD_THRESHOLD) mask |= NAV_DOWN;
    if (s_cpad_x <= -CPAD_THRESHOLD) mask |= NAV_LEFT;
    if (s_cpad_x >=  CPAD_THRESHOLD) mask |= NAV_RIGHT;
    return mask;
}

void input_update(void) {
    hidScanInput();
    uint32_t k_down = hidKeysDown();
    uint32_t k_held = hidKeysHeld();
    uint32_t k_up   = hidKeysUp();

    s_prev_held = s_held;
    s_held     = k_held;
    s_pressed  = k_down;
    s_released = k_up;

    /* Circle pad read. hidCircleRead fills circlePosition with
     * dx/dy in range approx -0x5C..0x5C (0 = centered). */
    hidCircleRead(&s_cpad);
    s_cpad_x = s_cpad.dx;
    s_cpad_y = s_cpad.dy;

    /* Touch. */
    uint8_t was_down = s_touch.touch_down;
    uint8_t is_down = (k_held & KEY_TOUCH) ? 1 : 0;

    s_touch.touch_pressed  = (is_down && !was_down) ? 1 : 0;
    s_touch.touch_released = (!is_down && was_down) ? 1 : 0;
    s_touch.touch_down    = is_down;

    s_touch.prev_tx = s_touch.tx;
    s_touch.prev_ty = s_touch.ty;
    if (is_down) {
        hidTouchRead(&s_cur_touch);
        s_touch.tx = (uint16_t)s_cur_touch.px;
        s_touch.ty = (uint16_t)s_cur_touch.py;
    }

    /* Double-tap detection within 250 ms. */
    if (s_touch.touch_released) {
        uint64_t now = osGetTime();
        if (s_last_tap_valid && (now - s_last_tap_time) <= 250) {
            s_touch.double_tap = 1;
            s_last_tap_valid = 0;
        } else {
            s_touch.double_tap = 0;
            s_last_tap_valid = 1;
            s_last_tap_time = now;
        }
    } else {
        s_touch.double_tap = 0;
    }

    /* Gamepad navigation with repeat timing. */
    uint8_t now_held = compute_nav_held();
    s_nav_pressed = 0;
    uint64_t now_ms = osGetTime();

    for (int i = 0; i < 4; ++i) {
        NavDirState* st = &s_nav_states[i];
        uint8_t active = (now_held & st->mask) ? 1 : 0;
        uint8_t was_active = (s_nav_held & st->mask) ? 1 : 0;

        if (active && !was_active) {
            /* Fresh press: fire immediately, start timer. */
            s_nav_pressed |= st->mask;
            st->hold_start = now_ms;
            st->last_repeat = now_ms;
        } else if (active && was_active) {
            /* Held: check for auto-repeat. */
            uint64_t held_duration = now_ms - st->hold_start;
            if (held_duration >= NAV_INITIAL_DELAY) {
                uint64_t since_last = now_ms - st->last_repeat;
                if (since_last >= NAV_REPEAT_INTERVAL) {
                    s_nav_pressed |= st->mask;
                    st->last_repeat = now_ms;
                }
            }
        } else if (!active) {
            st->hold_start = 0;
            st->last_repeat = 0;
        }
    }

    s_nav_held = now_held;
}

uint32_t input_held(uint32_t mask) {
    return s_held & mask;
}

uint32_t input_pressed(uint32_t mask) {
    return s_pressed & mask;
}

uint32_t input_released(uint32_t mask) {
    return s_released & mask;
}

const TouchState* input_touch(void) {
    return &s_touch;
}

uint8_t input_nav_pressed(void) {
    return s_nav_pressed;
}

uint8_t input_nav_held(void) {
    return s_nav_held;
}

int input_cpad_x(void) {
    return s_cpad_x;
}

int input_cpad_y(void) {
    return s_cpad_y;
}
