#ifndef SYCAMORE_LIBINPUT_H
#define SYCAMORE_LIBINPUT_H

#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_input_device.h>

void touchpad_set_natural_scroll(struct wlr_input_device *device);

void touchpad_set_tap_to_click(struct wlr_input_device *device);

void touchpad_set_accel_speed(struct wlr_input_device *device, double speed);

#endif //SYCAMORE_LIBINPUT_H
