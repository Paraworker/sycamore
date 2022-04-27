#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include "sycamore/input/cursor.h"

struct sycamore_seat;
struct sycamore_server;
struct sycamore_view;

enum seatop_mode {
    SEATOP_DEFAULT,
    SEATOP_POINTER_MOVE,
    SEATOP_POINTER_RESIZE,
};

struct sycamore_seatop_impl {
    void (*pointer_button)(struct sycamore_seat *seat, struct wlr_pointer_button_event *event);
    void (*pointer_motion)(struct sycamore_seat *seat, uint32_t time_msec);
    void (*cursor_rebase)(struct sycamore_seat *seat);
    void (*end)(struct sycamore_seat *seat);
    enum seatop_mode mode;
};

struct sycamore_seat_device {
    struct wl_list link; // sycamore_seat::devices
    struct wlr_input_device *wlr_device;

    union {
        void *derived_device;
        struct sycamore_pointer *pointer;
        struct sycamore_keyboard *keyboard;
    };

    void (*derived_destroy)(struct sycamore_seat_device *seat_device);

    struct wl_listener destroy;

    struct sycamore_seat *seat;
};

struct sycamore_drag_icon {
    struct sycamore_seat *seat;
    struct wlr_drag_icon *wlr_drag_icon;

    double x, y; // in layout-local coordinates

    struct wl_listener surface_commit;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
};

struct sycamore_drag {
    struct sycamore_seat *seat;
    struct wlr_drag *wlr_drag;
    struct wl_listener destroy;
};

struct sycamore_seat {
    struct wlr_seat *wlr_seat;
    struct sycamore_cursor *cursor;
    struct wl_list devices;

    const struct sycamore_seatop_impl *seatop_impl;
    void *seatop_data;

    struct wl_listener request_set_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener request_set_primary_selection;
    struct wl_listener request_start_drag;
    struct wl_listener start_drag;
    struct wl_listener destroy;

    struct sycamore_server *server;
};

void handle_backend_new_input(struct wl_listener *listener, void *data);

struct sycamore_seat *sycamore_seat_create(struct sycamore_server *server,
        struct wl_display *display, struct wlr_output_layout *output_layout);

void sycamore_seat_destroy(struct sycamore_seat *seat);

struct sycamore_seat_device *seat_device_create(struct sycamore_seat *seat, struct wlr_input_device *wlr_device,
        void *derived_device, void (*derived_destroy)(struct sycamore_seat_device *seat_device));

void seat_device_destroy(struct sycamore_seat_device *seat_device);

void seat_update_capabilities(struct sycamore_seat *seat);

void seatop_begin_default(struct sycamore_seat *seat);

void seatop_begin_pointer_move(struct sycamore_seat *seat, struct sycamore_view *view);

void seatop_begin_pointer_resize(struct sycamore_seat *seat, struct sycamore_view *view, uint32_t edges);

void seatop_end(struct sycamore_seat *seat);

bool seatop_interactive_assert(struct sycamore_seat *seat, struct sycamore_view *view);

#endif //SYCAMORE_SEAT_H
