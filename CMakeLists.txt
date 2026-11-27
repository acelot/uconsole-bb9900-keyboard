# Local ZMK module, picked up through zephyr/module.yml. See src/scroll_invert.c.
zephyr_library()
zephyr_library_sources(src/scroll_invert.c)

# The trackpad loop calls zmk_hid_mouse_scroll_update() directly, so the
# direction is changed at link time by wrapping that one function. Pointer
# movement is left alone.
zephyr_ld_options(-Wl,--wrap=zmk_hid_mouse_scroll_update)
