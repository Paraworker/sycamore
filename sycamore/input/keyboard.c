#include <stdlib.h>
#include <wlr/util/log.h>

#include "sycamore/input/keyboard.h"
#include "sycamore/input/keybinding.h"

static void handle_keyboard_modifiers(
        struct wl_listener *listener, void *data) {
    /* This event is raised when a modifier key, such as shift or alt, is
     * pressed or released. */
    struct sycamore_keyboard *keyboard =
            wl_container_of(listener, keyboard, modifiers);

    wlr_seat_set_keyboard(keyboard->seat->wlr_seat, keyboard->device);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(keyboard->seat->wlr_seat,
                                       &keyboard->device->keyboard->modifiers);
}

static void handle_keyboard_key(
        struct wl_listener *listener, void *data) {
    /* This event is raised when a key is pressed or released. */
    struct sycamore_keyboard *keyboard =
            wl_container_of(listener, keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = keyboard->seat->wlr_seat;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    /* Get a list of keysyms based on the keymap for this keyboard */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
            keyboard->device->keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        /* If this button was pressed, we attempt to
         * process it as a compositor keybinding. */
        uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
        for (int i = 0; i < nsyms; ++i) {
            handled = handle_keybinding(keyboard->seat->server->keybinding_manager,
                                        modifiers, syms[i]);
        }
    }

    if (!handled) {
        /* Otherwise, we pass it along to the client. */
        wlr_seat_set_keyboard(seat, keyboard->device);
        wlr_seat_keyboard_notify_key(seat, event->time_msec,
                                     event->keycode, event->state);
    }
}

static void handle_keyboard_destroy(struct wl_listener *listener, void *data) {
    /* This event is raised by the keyboard base wlr_input_device to signal
     * the destruction of the wlr_keyboard. It will no longer receive events
     * and should be destroyed.
     */
    struct sycamore_keyboard *keyboard =
            wl_container_of(listener, keyboard, destroy);

    sycamore_keyboard_destroy(keyboard);
}

void sycamore_keyboard_destroy(struct sycamore_keyboard* keyboard) {
    if (!keyboard) {
        return;
    }

    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);

    free(keyboard);
}

struct sycamore_keyboard* sycamore_keyboard_create(struct sycamore_seat* seat,
        struct wlr_input_device *device) {
    struct sycamore_keyboard *keyboard = calloc(1, sizeof(struct sycamore_keyboard));
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_keyboard");
        return NULL;
    }

    keyboard->seat = seat;
    keyboard->device = device;

    /* Prepare an XKB keymap and assign it to the keyboard. This
     * assumes the defaults (e.g. layout = "us"). */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
                                                          XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    /* Set up listeners for keyboard events. */
    keyboard->modifiers.notify = handle_keyboard_modifiers;
    wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = handle_keyboard_key;
    wl_signal_add(&device->keyboard->events.key, &keyboard->key);
    keyboard->destroy.notify = handle_keyboard_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(seat->wlr_seat, device);

    return keyboard;
}



