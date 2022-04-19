#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include "sycamore/input/seat.h"

struct sycamore_keyboard {
    struct sycamore_seat_device *base;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
};

struct sycamore_keyboard *sycamore_keyboard_create(struct sycamore_seat *seat,
        struct wlr_input_device *wlr_device);

#endif //SYCAMORE_KEYBOARD_H
