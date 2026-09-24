/**
 * Shared bottom-screen navigation.
 *
 * Each scene draws a fixed bottom-screen nav bar with 5 tabs
 * (Home / Apps / Downloads / Forum / Chat) and routes to the matching
 * scene via scene_manager_switch. The Settings button is reached
 * from the Home scene (gear icon).
 *
 * Supports both touch and gamepad (D-pad/circle pad + A button).
 */
#ifndef SCENES_NAV_H
#define SCENES_NAV_H

#include "../core/input.h"

/* Call from each scene's on_touch BEFORE its own widgets so nav taps
 * always take priority. Returns 1 if a nav tap was consumed. */
int nav_handle_touch(const TouchState* t);

/* Handle gamepad navigation (left/right to move focus, A to activate).
 * Returns 1 if a gamepad event was consumed. */
int nav_handle_gamepad(void);

/* Current focused nav index (0-4). */
int nav_focus_index(void);

/* Set the focused nav index. */
void nav_set_focus(int idx);

/* Draw the nav bar at the bottom of the given screen height.
 * active_index: 0=home 1=apps 2=downloads 3=forum 4=chat. */
void nav_draw(int active_index);

#endif /* SCENES_NAV_H */
