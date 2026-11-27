/*
 * SPDX-License-Identifier: MIT
 */

/*
 * Scroll fixes for the local ZMK module: axis lock and direction reversal.
 *
 * jhob101/zmk builds the scroll deltas in app/src/main.c and hands them to
 * zmk_hid_mouse_scroll_update(). That fork has no setting for either
 * behaviour, so this module wraps the function at link time (see
 * CMakeLists.txt) and changes the deltas here.
 * zmk_hid_mouse_movement_update() is not wrapped, so the pointer keeps its
 * own direction and axis.
 */

#include <zephyr/kernel.h>
#include <stdbool.h>

void __real_zmk_hid_mouse_scroll_update(int8_t x, int8_t y);

/* Provided by jhob101/zmk's app/src/behaviors/behavior_scroll_hold.c. */
bool zmk_trackpad_scroll_held(void);

#ifdef CONFIG_BB9900_SCROLL_AXIS_LOCK

enum scroll_axis {
    SCROLL_AXIS_NONE,
    SCROLL_AXIS_HORIZONTAL,
    SCROLL_AXIS_VERTICAL,
};

// How long scrolling must stand still before the next movement may pick a
// different axis. A scroll switch can stay on between gestures, so the key
// being up is not enough on its own; within one gesture, a pause this long is
// treated as starting a new one.
#define SCROLL_AXIS_IDLE_MS 300

static enum scroll_axis locked_axis = SCROLL_AXIS_NONE;
static int64_t last_scroll_ms;

static int scroll_abs(int value) { return value < 0 ? -value : value; }

#endif

void __wrap_zmk_hid_mouse_scroll_update(int8_t x, int8_t y) {
#ifdef CONFIG_BB9900_SCROLL_AXIS_LOCK
    const bool moving = x != 0 || y != 0;
    const int64_t now = k_uptime_get();
    const bool axis_expired =
        locked_axis != SCROLL_AXIS_NONE && now - last_scroll_ms > SCROLL_AXIS_IDLE_MS;

    if (moving) {
        if (!zmk_trackpad_scroll_held() || locked_axis == SCROLL_AXIS_NONE || axis_expired) {
            // Vertical wins a tie, and any diagonal that is not clearly
            // horizontal, so a slightly sloppy up/down drag stays vertical.
            locked_axis = (scroll_abs(x) > scroll_abs(y)) ? SCROLL_AXIS_HORIZONTAL
                                                         : SCROLL_AXIS_VERTICAL;
        }
        last_scroll_ms = now;
        if (locked_axis == SCROLL_AXIS_HORIZONTAL) {
            y = 0;
        } else {
            x = 0;
        }
    } else if (!zmk_trackpad_scroll_held() || axis_expired) {
        locked_axis = SCROLL_AXIS_NONE;
    }
#endif

#ifdef CONFIG_BB9900_SCROLL_INVERT_X
    x = (int8_t)-x;
#endif
#ifdef CONFIG_BB9900_SCROLL_INVERT_Y
    y = (int8_t)-y;
#endif

    __real_zmk_hid_mouse_scroll_update(x, y);
}
