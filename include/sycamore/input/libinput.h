#ifndef SYCAMORE_LIBINPUT_H
#define SYCAMORE_LIBINPUT_H

#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_input_device.h>

void touchpadSetNaturalScroll(struct wlr_input_device *device);

void touchpadSetTapToClick(struct wlr_input_device *device);

void touchpadSetAccelSpeed(struct wlr_input_device *device, double speed);

#endif //SYCAMORE_LIBINPUT_H