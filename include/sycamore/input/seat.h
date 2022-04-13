#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include "sycamore/input/cursor.h"
#include "sycamore/server.h"

struct sycamore_seat {
    struct wlr_seat *wlr_seat;

    struct wl_list devices;
    struct sycamore_cursor *cursor;

    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener destroy;

    struct sycamore_server *server;
};

struct sycamore_seat_device {
    struct wl_list link; // sycamore_seat::devices
    struct wlr_input_device *wlr_device;

    union {
        void *type;
        struct sycamore_keyboard *keyboard;
    };

    struct wl_listener destroy;

    struct sycamore_seat *seat;
};

struct sycamore_seat *sycamore_seat_create(struct sycamore_server *server,
        struct wl_display *display, struct wlr_output_layout *output_layout);

void sycamore_seat_destroy(struct sycamore_seat *seat);

struct sycamore_seat_device *seat_device_create(struct sycamore_seat *seat, struct wlr_input_device *wlr_device,
        void *type, void(*handle_destroy)(struct wl_listener *listener, void *data));

void seat_device_destroy(struct sycamore_seat_device *seat_device);

void seat_update_capabilities(struct sycamore_seat *seat);

void handle_backend_new_input(struct wl_listener *listener, void *data);

#endif //SYCAMORE_SEAT_H
