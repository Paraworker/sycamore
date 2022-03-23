#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/util/log.h>


#include "sycamore/server.h"
#include "sycamore/input/seat.h"
#include "sycamore/input/keyboard.h"
#include "sycamore/input/libinput.h"

static void new_pointer(struct sycamore_seat *seat,
        struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new pointer device: %s", device->name);
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor, device);

    touchpad_set_tap_to_click(device);
    touchpad_set_natural_scroll(device);
    touchpad_set_accel_speed(device, 0.3);
}

static void new_keyboard(struct sycamore_seat *seat,
                                struct wlr_input_device *device) {

    struct sycamore_keyboard* keyboard = sycamore_keyboard_create(seat, device);
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to create keyboard");
        return;
    }

    wl_list_insert(&seat->keyboards, &keyboard->link);
}

static void new_touch(struct sycamore_seat *seat,
                             struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new touch device: %s", device->name);
    /* TODO */
}

static void new_tablet_tool(struct sycamore_seat *seat,
                                   struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet tool device: %s", device->name);
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor, device);
    /* TODO */
}

static void new_tablet_pad(struct sycamore_seat *seat,
                                  struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet pad device: %s", device->name);
    /* TODO */
}

static void new_switch(struct sycamore_seat *seat,
                              struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new switch device: %s", device->name);
    /* TODO */
}

void handle_backend_new_input(struct wl_listener *listener, void *data) {
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct sycamore_server *server =
            wl_container_of(listener, server, backend_new_input);
    struct sycamore_seat* seat = server->seat;
    struct wlr_input_device *device = data;
    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            new_keyboard(seat, device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            new_pointer(seat, device);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            new_touch(seat, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            new_tablet_tool(seat, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
            new_tablet_pad(seat, device);
            break;
        case WLR_INPUT_DEVICE_SWITCH:
            new_switch(seat, device);
            break;

        default:
            break;
    }
    /* We need to let the wlr_seat know what our capabilities are, which is
     * communiciated to the client. In Sycamore we always have a cursor, even if
     * there are no pointer devices, so we always include that capability. */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&seat->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(seat->wlr_seat, caps);
}

static void handle_seat_request_cursor(struct wl_listener *listener, void *data) {
    struct sycamore_seat *seat = wl_container_of(
            listener, seat, request_cursor);
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
    struct sycamore_seat* seat = wl_container_of(listener, seat, destroy);

    seat->wlr_seat = NULL;
    seat->server->seat = NULL;
    sycamore_seat_destroy(seat);
}

void sycamore_seat_destroy(struct sycamore_seat* seat) {
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

struct sycamore_seat* sycamore_seat_create(struct sycamore_server* server,
        struct wl_display* display, struct wlr_output_layout* output_layout) {
    struct sycamore_seat* seat = calloc(1, sizeof(struct sycamore_seat));
    if (!seat) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_seat");
        return NULL;
    }

    seat->server = server;

    seat->wlr_seat = wlr_seat_create(display, "seat0");
    if (!seat->wlr_seat) {
        wlr_log(WLR_ERROR, "Unable to create wlr_seat");
        sycamore_seat_destroy(seat);
        return NULL;
    }

    seat->cursor = sycamore_cursor_create(seat, output_layout);
    if (!seat->cursor) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_cursor");
        sycamore_seat_destroy(seat);
        return NULL;
    }

    wl_list_init(&seat->keyboards);

    seat->request_cursor.notify = handle_seat_request_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor,
                  &seat->request_cursor);
    seat->request_set_selection.notify = handle_seat_request_set_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_selection,
                  &seat->request_set_selection);
    seat->destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->wlr_seat->events.destroy,
                  &seat->destroy);

    return seat;
}

