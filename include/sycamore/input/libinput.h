#ifndef SYCAMORE_LIBINPUT_H
#define SYCAMORE_LIBINPUT_H

#include "sycamore/wlroots.h"

void touchpadSetNaturalScroll(wlr_input_device* device);

void touchpadSetTapToClick(wlr_input_device* device);

void touchpadSetAccelSpeed(wlr_input_device* device, double speed);

#endif //SYCAMORE_LIBINPUT_H