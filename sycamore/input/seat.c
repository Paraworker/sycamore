#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_data_device.h>

#include "sycamore/server.h"
#include "sycamore/input/seat.h"
#include "sycamore/input/keyboard.h"
#include "sycamore/input/libinput.h"

void new_pointer(struct sycamore_server *server,
        struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new pointer device: %s", device->name);
    wlr_cursor_attach_input_device(server->seat->cursor->wlr_cursor, device);

    set_touchpad_tap_to_click(device);
    set_touchpad_natural_scroll(device);
}

static void new_keyboard(struct sycamore_server *server,
                                struct wlr_input_device *device) {

    struct sycamore_keyboard* keyboard = sycamore_keyboard_create(server->seat, device);
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to create keyboard");
        return;
    }
}

static void new_touch(struct sycamore_server *server,
                             struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new touch device: %s", device->name);
    /* to do */
}

static void new_tablet_tool(struct sycamore_server *server,
                                   struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet tool device: %s", device->name);
    wlr_cursor_attach_input_device(server->seat->cursor->wlr_cursor, device);
    /* to do */
}

static void new_tablet_pad(struct sycamore_server *server,
                                  struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet pad device: %s", device->name);
    /* to do */
}

static void new_switch(struct sycamore_server *server,
                              struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new switch device: %s", device->name);
    /* to do */
}

void handle_backend_new_input(struct wl_listener *listener, void *data) {
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct sycamore_server *server =
            wl_container_of(listener, server, backend_new_input);
    struct wlr_input_device *device = data;
    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            new_keyboard(server, device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            new_pointer(server, device);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            new_touch(server, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            new_tablet_tool(server, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
            new_tablet_pad(server, device);
            break;
        case WLR_INPUT_DEVICE_SWITCH:
            new_switch(server, device);
            break;

        default:
            break;
    }
    /* We need to let the wlr_seat know what our capabilities are, which is
     * communiciated to the client. In Sycamore we always have a cursor, even if
     * there are no pointer devices, so we always include that capability. */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->seat->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat->wlr_seat, caps);
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
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

static void seat_request_set_selection(struct wl_listener *listener, void *data) {
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in sycamore we always honor
     */
    struct sycamore_seat *seat = wl_container_of(
            listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(seat->wlr_seat, event->source, event->serial);
}

void sycamore_seat_destroy(struct sycamore_seat* seat) {
    if (!seat) {
        return;
    }

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

    seat->request_cursor.notify = seat_request_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor,
                  &seat->request_cursor);
    seat->request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_selection,
                  &seat->request_set_selection);

    return seat;
}

