#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include "sycamore/input/cursor.h"
#include "sycamore/server.h"

enum seatop_mode {
    SEATOP_DEFAULT,
    SEATOP_POINTER_MOVE,
    SEATOP_POINTER_RESIZE,
};

struct sycamore_seat;

struct sycamore_seatop_impl {
    void (*pointer_button)(struct sycamore_seat *seat, struct wlr_pointer_button_event *event);
    void (*pointer_motion)(struct sycamore_seat *seat, uint32_t time_msec);
    enum seatop_mode mode;
};

struct sycamore_seat {
    struct wlr_seat *wlr_seat;
    const struct sycamore_seatop_impl *seatop_impl;
    struct sycamore_cursor *cursor;
    struct wl_list devices;

    struct sycamore_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener destroy;

    struct sycamore_server *server;
};

struct sycamore_seat_device {
    struct wl_list link; // sycamore_seat::devices
    struct wlr_input_device *wlr_device;

    union {
        void *derived_device;
        struct sycamore_pointer *pointer;
        struct sycamore_keyboard *keyboard;
    };

    struct wl_listener destroy;

    struct sycamore_seat *seat;
};

void handle_backend_new_input(struct wl_listener *listener, void *data);

struct sycamore_seat *sycamore_seat_create(struct sycamore_server *server,
        struct wl_display *display, struct wlr_output_layout *output_layout);

void sycamore_seat_destroy(struct sycamore_seat *seat);

struct sycamore_seat_device *seat_device_create(struct sycamore_seat *seat, struct wlr_input_device *wlr_device,
        void *derived_device, void(*handle_destroy)(struct wl_listener *listener, void *data));

void seat_device_destroy(struct sycamore_seat_device *seat_device);

void seat_update_capabilities(struct sycamore_seat *seat);

void seatop_begin_default(struct sycamore_seat* seat);

void seatop_begin_pointer_move(struct sycamore_seat* seat, struct sycamore_view *view);

void seatop_begin_pointer_resize(struct sycamore_seat* seat, struct sycamore_view *view, uint32_t edges);

#endif //SYCAMORE_SEAT_H
