/**
 * Input abstraction over HID (buttons), circle pad and touch panel.
 * Each frame call input_update() once; then read state with helpers.
 *
 * Gamepad navigation: D-pad and circle pad generate directional
 * "nav" events (up/down/left/right) with repeat timing so scenes can
 * navigate lists and grids without touch.
 */
#ifndef CORE_INPUT_H
#define CORE_INPUT_H

#include <3ds.h>
#include <stdint.h>

/*---- Button edge events over the HID mask. ----*/
#define KEY_A_3DS       KEY_A
#define KEY_B_3DS       KEY_B
#define KEY_X_3DS       KEY_X
#define KEY_Y_3DS       KEY_Y
#define KEY_L_3DS        KEY_L
#define KEY_R_3DS        KEY_R
#define KEY_DUP_3DS      KEY_DUP
#define KEY_DDOWN_3DS    KEY_DDOWN
#define KEY_DLEFT_3DS    KEY_DLEFT
#define KEY_DRIGHT_3DS   KEY_DRIGHT
#define KEY_START_3DS    KEY_START
#define KEY_SELECT_3DS   KEY_SELECT
#define KEY_CPAD_UP      (1 << 24)
#define KEY_CPAD_DOWN    (1 << 25)
#define KEY_CPAD_LEFT    (1 << 26)
#define KEY_CPAD_RIGHT   (1 << 27)

/* Combined directional mask (D-pad OR circle pad). */
#define NAV_UP    0x01
#define NAV_DOWN  0x02
#define NAV_LEFT  0x04
#define NAV_RIGHT 0x08

typedef struct {
    /* Touch position in bottom-screen pixels (0..319, 0..239) */
    uint16_t tx, ty;
    uint8_t  touch_down;     /* held this frame */
    uint8_t  touch_pressed;  /* edge: down now, was up */
    uint8_t  touch_released; /* edge: up now, was down */
    uint16_t prev_tx, prev_ty;
    uint8_t  double_tap;     /* set on second tap within 250ms */
} TouchState;

/*---- API ----*/
void input_init(void);

/* Call exactly once per frame BEFORE scene update. Reads HID + touch. */
void input_update(void);

/* Held (continuous) state. */
uint32_t input_held(uint32_t mask);

/* Pressed (rising edge this frame). */
uint32_t input_pressed(uint32_t mask);

/* Released (falling edge this frame). */
uint32_t input_released(uint32_t mask);

const TouchState* input_touch(void);

/*---- Gamepad navigation ----*/

/* Returns a bitmask of NAV_UP/NAV_DOWN/NAV_LEFT/NAV_RIGHT for directions
 * that were "tapped" this frame (rising edge). Includes D-pad and
 * circle-pad deflection beyond a deadzone. */
uint8_t input_nav_pressed(void);

/* Returns a bitmask of directions currently held (continuous). */
uint8_t input_nav_held(void);

/* Circle pad X/Y (-0x5C..0x5C typical). 0 = centered. */
int input_cpad_x(void);
int input_cpad_y(void);

#endif /* CORE_INPUT_H */
