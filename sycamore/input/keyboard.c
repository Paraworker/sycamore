#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/input/keyboard.h"
#include "sycamore/input/keybinding.h"

static void handle_keyboard_modifiers(struct wl_listener *listener, void *data) {
    /* This event is raised when a modifier key, such as shift or alt, is
     * pressed or released. */
    struct sycamore_keyboard *keyboard =
            wl_container_of(listener, keyboard, modifiers);

    wlr_seat_set_keyboard(keyboard->base->seat->wlr_seat, keyboard->wlr_keyboard);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(keyboard->base->seat->wlr_seat,
                                       &keyboard->wlr_keyboard->modifiers);
}

static void handle_keyboard_key(struct wl_listener *listener, void *data) {
    /* This event is raised when a key is pressed or released. */
    struct sycamore_keyboard *keyboard =
            wl_container_of(listener, keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = keyboard->base->seat->wlr_seat;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    /* Get a list of keysyms based on the keymap for this keyboard */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
            keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        /* If this button was pressed, we attempt to
         * process it as a compositor keybinding. */
        uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
        for (int i = 0; i < nsyms; ++i) {
            handled = handle_keybinding(keyboard->base->seat->server->keybinding_manager,
                                        modifiers, syms[i]);
        }
    }

    if (!handled) {
        /* Otherwise, we pass it along to the client. */
        wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec,
                                     event->keycode, event->state);
    }
}

static void handle_keyboard_destroy(struct wl_listener *listener, void *data) {
    /* This event is raised by the keyboard base wlr_input_device to signal
     * the destruction of the wlr_keyboard. It will no longer receive events
     * and should be destroyed.
     */
    struct sycamore_seat_device *device = wl_container_of(listener, device, destroy);
    struct sycamore_seat *seat = device->seat;
    sycamore_keyboard_destroy(device->keyboard);
    seat_update_capabilities(seat);
}

void sycamore_keyboard_destroy(struct sycamore_keyboard *keyboard) {
    if (!keyboard) {
        return;
    }

    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);

    if (keyboard->base) {
        seat_device_destroy(keyboard->base);
    }

    free(keyboard);
}

struct sycamore_keyboard *sycamore_keyboard_create(struct sycamore_seat *seat,
        struct wlr_input_device *wlr_device) {
    struct sycamore_keyboard *keyboard = calloc(1, sizeof(struct sycamore_keyboard));
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_keyboard");
        return NULL;
    }

    keyboard->base = seat_device_create(seat, wlr_device, keyboard,
                                        handle_keyboard_destroy);
    if (!keyboard->base) {
        wlr_log(WLR_ERROR, "Unable to create seat_device");
        free(keyboard);
        return NULL;
    }

    keyboard->wlr_keyboard = wlr_device->keyboard;

    /* Prepare an XKB keymap and assign it to the keyboard. This
     * assumes the defaults (e.g. layout = "us"). */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
                                                          XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    keyboard->modifiers.notify = handle_keyboard_modifiers;
    wl_signal_add(&keyboard->wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = handle_keyboard_key;
    wl_signal_add(&keyboard->wlr_keyboard->events.key, &keyboard->key);

    return keyboard;
}



