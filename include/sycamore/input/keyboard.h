#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include "sycamore/server.h"
#include "sycamore/input/seat.h"

struct sycamore_keyboard {
    struct wl_list link;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;

    struct sycamore_seat *seat;
};

struct sycamore_keyboard *sycamore_keyboard_create(struct sycamore_seat *seat,
        struct wlr_input_device *device);

void sycamore_keyboard_destroy(struct sycamore_keyboard *keyboard);

#endif //SYCAMORE_KEYBOARD_H
