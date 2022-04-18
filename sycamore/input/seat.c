#include <stdlib.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "sycamore/server.h"
#include "sycamore/input/seat.h"
#include "sycamore/input/keyboard.h"
#include "sycamore/input/libinput.h"

struct sycamore_seat_device *seat_device_create(struct sycamore_seat *seat, struct wlr_input_device *wlr_device,
        void *derived_device, void(*handle_destroy)(struct wl_listener *listener, void *data)) {
    struct sycamore_seat_device *seat_device =
            calloc(1, sizeof(struct sycamore_seat_device));
    if (!seat_device) {
        return NULL;
    }

    seat_device->wlr_device = wlr_device;
    seat_device->derived_device = derived_device;
    seat_device->seat = seat;

    seat_device->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_device->events.destroy, &seat_device->destroy);

    return seat_device;
}

void seat_device_destroy(struct sycamore_seat_device *seat_device) {
    if (!seat_device) {
        return;
    }

    wl_list_remove(&seat_device->destroy.link);
    wl_list_remove(&seat_device->link);

    free(seat_device);
}

void handle_seat_device_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_seat_device *seat_device = wl_container_of(listener, seat_device, destroy);
    struct sycamore_seat *seat = seat_device->seat;
    seat_device_destroy(seat_device);
    seat_update_capabilities(seat);
}

void seat_update_capabilities(struct sycamore_seat *seat) {
    uint32_t caps = 0;
    struct sycamore_seat_device *seat_device;
    wl_list_for_each(seat_device, &seat->devices, link) {
        switch (seat_device->wlr_device->type) {
            case WLR_INPUT_DEVICE_KEYBOARD:
                caps |= WL_SEAT_CAPABILITY_KEYBOARD;
                break;
            case WLR_INPUT_DEVICE_POINTER:
                caps |= WL_SEAT_CAPABILITY_POINTER;
                break;
            case WLR_INPUT_DEVICE_TOUCH:
                caps |= WL_SEAT_CAPABILITY_TOUCH;
                break;
            case WLR_INPUT_DEVICE_TABLET_TOOL:
                caps |= WL_SEAT_CAPABILITY_POINTER;
                break;
            case WLR_INPUT_DEVICE_SWITCH:
            case WLR_INPUT_DEVICE_TABLET_PAD:
                break;
        }
    }

    wlr_seat_set_capabilities(seat->wlr_seat, caps);

    // Disable cursor if seat doesn't have pointer capability.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
        cursor_disable(seat->cursor);
    }
}

static void seat_configure_pointer(struct sycamore_seat *seat,
                                   struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new pointer device: %s", device->name);

    struct sycamore_seat_device* seat_device = seat_device_create(seat, device,
            NULL, handle_seat_device_destroy);
    if (!seat_device) {
        wlr_log(WLR_ERROR, "Unable to create seat_device");
        return;
    }

    wl_list_insert(&seat->devices, &seat_device->link);
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor, device);

    touchpad_set_tap_to_click(device);
    touchpad_set_natural_scroll(device);
    touchpad_set_accel_speed(device, 0.3);
}

static void seat_configure_keyboard(struct sycamore_seat *seat,
                                    struct wlr_input_device *device) {
    struct sycamore_keyboard *keyboard = sycamore_keyboard_create(seat, device);
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to create keyboard");
        return;
    }

    wl_list_insert(&seat->devices, &keyboard->base->link);
    wlr_seat_set_keyboard(seat->wlr_seat, keyboard->wlr_keyboard);
}

static void seat_configure_touch(struct sycamore_seat *seat,
                                 struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new touch device: %s", device->name);
    /* TODO */
}

static void seat_configure_tablet_tool(struct sycamore_seat *seat,
                                       struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet tool device: %s", device->name);
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor, device);
    /* TODO */
}

static void seat_configure_tablet_pad(struct sycamore_seat *seat,
                                      struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet pad device: %s", device->name);
    /* TODO */
}

static void seat_configure_switch(struct sycamore_seat *seat,
                                  struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new switch device: %s", device->name);
    /* TODO */
}

void handle_backend_new_input(struct wl_listener *listener, void *data) {
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct sycamore_server *server =
            wl_container_of(listener, server, backend_new_input);
    struct wlr_input_device *device = data;
    struct sycamore_seat *seat = server->seat;
    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            seat_configure_keyboard(seat, device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            seat_configure_pointer(seat, device);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            seat_configure_touch(seat, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            seat_configure_tablet_tool(seat, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
            seat_configure_tablet_pad(seat, device);
            break;
        case WLR_INPUT_DEVICE_SWITCH:
            seat_configure_switch(seat, device);
            break;

        default:
            break;
    }

    seat_update_capabilities(seat);
}

static void handle_seat_request_cursor(struct wl_listener *listener, void *data) {
    struct sycamore_seat *seat = wl_container_of(listener, seat, request_cursor);
    /* This event is raised by the seat when a client provides a cursor image */
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused_client =
            seat->wlr_seat->pointer_state.focused_client;
    /* This can be sent by any client, so we check to make sure this one is
     * actually has pointer focus first. */
    if (focused_client == event->seat_client) {
        /* Once we've vetted the client, we can tell the cursor to use the
         * provided surface as the cursor image. It will set the hardware cursor
         * on the output that it's currently on and continue to do so as the
         * cursor moves between outputs. */
        wlr_cursor_set_surface(seat->cursor->wlr_cursor, event->surface,
                               event->hotspot_x, event->hotspot_y);
    }
}

static void handle_seat_request_set_selection(struct wl_listener *listener, void *data) {
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in sycamore we always honor
     */
    struct sycamore_seat *seat = wl_container_of(
            listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(seat->wlr_seat, event->source, event->serial);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_seat *seat = wl_container_of(listener, seat, destroy);

    seat->wlr_seat = NULL;
    seat->server->seat = NULL;
    sycamore_seat_destroy(seat);
}

void sycamore_seat_destroy(struct sycamore_seat *seat) {
    if (!seat) {
        return;
    }

    wl_list_remove(&seat->request_cursor.link);
    wl_list_remove(&seat->request_set_selection.link);
    wl_list_remove(&seat->destroy.link);

    if (seat->wlr_seat) {
        wlr_seat_destroy(seat->wlr_seat);
    }
    if (seat->cursor) {
        sycamore_cursor_destroy(seat->cursor);
    }

    free(seat);
}

struct sycamore_seat *sycamore_seat_create(struct sycamore_server *server,
        struct wl_display *display, struct wlr_output_layout *output_layout) {
    struct sycamore_seat *seat = calloc(1, sizeof(struct sycamore_seat));
    if (!seat) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_seat");
        return NULL;
    }

    seat->wlr_seat = wlr_seat_create(display, "seat0");
    if (!seat->wlr_seat) {
        wlr_log(WLR_ERROR, "Unable to create wlr_seat");
        free(seat);
        return NULL;
    }

    seat->cursor = sycamore_cursor_create(seat, display, output_layout);
    if (!seat->cursor) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_cursor");
        sycamore_seat_destroy(seat);
        return NULL;
    }

    wl_list_init(&seat->devices);
    seat->server = server;

    seat->grabbed_view = NULL;

    seat->request_cursor.notify = handle_seat_request_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor, &seat->request_cursor);
    seat->request_set_selection.notify = handle_seat_request_set_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_selection, &seat->request_set_selection);
    seat->destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->wlr_seat->events.destroy, &seat->destroy);

    seatop_begin_default(seat);

    return seat;
}

