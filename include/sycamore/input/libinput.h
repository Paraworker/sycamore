#ifndef SYCAMORE_LIBINPUT_H
#define SYCAMORE_LIBINPUT_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/backend/libinput.h>

/* set touchpad natural scroll */
void set_touchpad_natural_scroll(struct wlr_input_device* device);

/* set touchpad tap to click */
void set_touchpad_tap_to_click(struct wlr_input_device* device);
#endif //SYCAMORE_LIBINPUT_H
