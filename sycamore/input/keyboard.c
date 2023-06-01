#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/input/seat.h"
#include "sycamore/input/seat_device.h"
#include "sycamore/input/keyboard.h"
#include "sycamore/input/keybinding.h"
#include "sycamore/server.h"

static void onKeyboardModifiers(struct wl_listener *listener, void *data) {
    /* This event is raised when a modifier key, such as shift or alt, is
     * pressed or released. */
    Keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);

    wlr_seat_set_keyboard(keyboard->base->seat->wlrSeat, keyboard->wlrKeyboard);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(keyboard->base->seat->wlrSeat,
                                       &keyboard->wlrKeyboard->modifiers);
}

static void onKeyboardKey(struct wl_listener *listener, void *data) {
    /* This event is raised when a key is pressed or released. */
    Keyboard *keyboard = wl_container_of(listener, keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = keyboard->base->seat->wlrSeat;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    /* Get a list of keysyms based on the keymap for this keyboard */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
            keyboard->wlrKeyboard->xkb_state, keycode, &syms);

    bool handled = false;
    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        /* If this button was pressed, we attempt to
         * process it as a compositor keybinding. */
        uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlrKeyboard);
        for (int i = 0; i < nsyms; ++i) {
            handled = handleKeybinding(server.keybindingManager, modifiers, syms[i]);
        }
    }

    if (!handled) {
        /* Otherwise, we pass it along to the client. */
        wlr_seat_set_keyboard(seat, keyboard->wlrKeyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec,
                                     event->keycode, event->state);
    }
}

static void keyboardDestroy(SeatDevice *seatDevice) {
    if (!seatDevice) {
        return;
    }

    wl_list_remove(&seatDevice->keyboard->modifiers.link);
    wl_list_remove(&seatDevice->keyboard->key.link);

    struct wlr_seat *wlrSeat = seatDevice->seat->wlrSeat;
    struct wlr_keyboard *wlrKeyboard = seatDevice->keyboard->wlrKeyboard;
    if (wlr_seat_get_keyboard(wlrSeat) == wlrKeyboard) {
        wlr_seat_set_keyboard(wlrSeat, nullptr);
    }

    free(seatDevice->keyboard);
}

Keyboard *keyboardCreate(Seat *seat, struct wlr_input_device *wlrDevice) {
    Keyboard *keyboard = calloc(1, sizeof(Keyboard));
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to allocate Keyboard");
        return nullptr;
    }

    keyboard->base = seatDeviceCreate(seat, wlrDevice, keyboard, keyboardDestroy);
    if (!keyboard->base) {
        wlr_log(WLR_ERROR, "Unable to create seatDevice");
        free(keyboard);
        return nullptr;
    }

    keyboard->wlrKeyboard = wlr_keyboard_from_input_device(wlrDevice);

    keyboard->modifiers.notify = onKeyboardModifiers;
    wl_signal_add(&keyboard->wlrKeyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = onKeyboardKey;
    wl_signal_add(&keyboard->wlrKeyboard->events.key, &keyboard->key);

    return keyboard;
}

static struct xkb_keymap *keyboardCompileKeymap() {
    /* Compile an XKB keymap
     * We assumes the defaults right now (e.g. layout = "us"). */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!context) {
        wlr_log(WLR_ERROR, "Unable to create xkb_context");
        return nullptr;
    }

    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, nullptr,
                                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkb_context_unref(context);
    return keymap;
}

void keyboardConfigure(Keyboard *keyboard) {
    struct xkb_keymap *keymap = keyboardCompileKeymap();
    if (!keymap) {
        wlr_log(WLR_ERROR, "Unable to compile xkb_keymap");
        return;
    }

    wlr_keyboard_set_keymap(keyboard->wlrKeyboard, keymap);
    xkb_keymap_unref(keymap);
}