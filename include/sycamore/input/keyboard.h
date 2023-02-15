#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include <wlr/types/wlr_keyboard.h>

typedef struct Keyboard   Keyboard;
typedef struct Seat       Seat;
typedef struct SeatDevice SeatDevice;

struct Keyboard {
    SeatDevice          *base;
    struct wlr_keyboard *wlrKeyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
};

Keyboard *keyboardCreate(Seat *seat, struct wlr_input_device *wlrDevice);

void keyboardConfigure(Keyboard *keyboard);

#endif //SYCAMORE_KEYBOARD_H