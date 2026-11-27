/*
 * SPDX-License-Identifier: MIT
 */

/*
 * Reverses the trackpad's scroll direction for the local ZMK module.
 *
 * jhob101/zmk builds the scroll deltas in app/src/main.c and hands them to
 * zmk_hid_mouse_scroll_update(). That fork has no setting for the direction,
 * so this module wraps the function at link time (see CMakeLists.txt) and
 * negates the deltas. zmk_hid_mouse_movement_update() is not wrapped, so the
 * pointer keeps its own direction.
 */

#include <zephyr/kernel.h>

void __real_zmk_hid_mouse_scroll_update(int8_t x, int8_t y);

void __wrap_zmk_hid_mouse_scroll_update(int8_t x, int8_t y) {
#ifdef CONFIG_BB9900_SCROLL_INVERT_X
    x = (int8_t)-x;
#endif
#ifdef CONFIG_BB9900_SCROLL_INVERT_Y
    y = (int8_t)-y;
#endif
    __real_zmk_hid_mouse_scroll_update(x, y);
}
