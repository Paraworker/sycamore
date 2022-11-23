#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"

struct sycamore_layer;
struct sycamore_seat;
struct sycamore_seat_device;

typedef void (*derived_seat_device_destroy)(struct sycamore_seat_device *seat_device);

enum seatop_mode {
    BLANK,
    BASIC_FULL,
    BASIC_POINTER_NO,
    POINTER_DOWN,
    POINTER_MOVE,
    POINTER_RESIZE,
};

struct seatop_pointer_down_data {
    struct wlr_surface *surface;
    struct wl_listener surface_destroy;
    double dx, dy;
    struct sycamore_seat *seat;
};

struct seatop_pointer_move_data {
    struct view_ptr view_ptr;
    double dx, dy;
};

struct seatop_pointer_resize_data {
    struct view_ptr view_ptr;
    double dx, dy;
    struct wlr_box grab_geobox;
    uint32_t edges;
};

union seatop_data {
    struct seatop_pointer_down_data pointer_down;
    struct seatop_pointer_move_data pointer_move;
    struct seatop_pointer_resize_data pointer_resize;
};

struct seatop_impl {
    void (*pointer_button)(struct sycamore_seat *seat, struct wlr_pointer_button_event *event);
    void (*pointer_motion)(struct sycamore_seat *seat, uint32_t time_msec);
    void (*pointer_rebase)(struct sycamore_seat *seat);
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

    derived_seat_device_destroy derived_destroy;

    struct wl_listener destroy;

    struct sycamore_seat *seat;
};

struct sycamore_drag_icon {
    struct wlr_drag_icon *wlr_drag_icon;

    struct wlr_scene_tree *tree;
    struct wlr_scene_tree *surface_tree;

    struct wl_listener commit;
    struct wl_listener destroy;
};

struct sycamore_drag {
    struct wlr_drag *wlr_drag;
    struct wl_listener destroy;
    struct sycamore_seat *seat;
};

struct sycamore_seat {
    struct wlr_seat *wlr_seat;
    struct sycamore_cursor *cursor;
    struct wl_list devices;

    const struct seatop_impl *seatop_impl;
    union seatop_data seatop_data;

    struct sycamore_layer *focused_layer;

    struct wl_listener request_set_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener request_set_primary_selection;
    struct wl_listener request_start_drag;
    struct wl_listener start_drag;
    struct wl_listener destroy;
};

void handle_backend_new_input(struct wl_listener *listener, void *data);

struct sycamore_seat *sycamore_seat_create(struct wl_display *display, struct wlr_output_layout *layout);

void sycamore_seat_destroy(struct sycamore_seat *seat);

struct sycamore_seat_device *seat_device_create(struct sycamore_seat *seat, struct wlr_input_device *wlr_device,
        void *derived_device, derived_seat_device_destroy derived_destroy);

void seat_device_destroy(struct sycamore_seat_device *seat_device);

void seat_update_capabilities(struct sycamore_seat *seat);

void seat_set_keyboard_focus(struct sycamore_seat *seat, struct wlr_surface *surface);

void seat_drag_icon_update_position(struct sycamore_seat *seat, struct sycamore_drag_icon *icon);

static inline void pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    if (seat->seatop_impl->pointer_button) {
        seat->seatop_impl->pointer_button(seat, event);
    }
}

static inline void pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    if (seat->seatop_impl->pointer_motion) {
        seat->seatop_impl->pointer_motion(seat, time_msec);
    }
}

static inline void pointer_rebase(struct sycamore_seat *seat) {
    if (seat->seatop_impl->pointer_rebase) {
        seat->seatop_impl->pointer_rebase(seat);
    }
}

static inline void seatop_end(struct sycamore_seat *seat) {
    if (seat->seatop_impl && seat->seatop_impl->end) {
        seat->seatop_impl->end(seat);
    }

    seat->seatop_impl = NULL;
}

void seatop_set_blank(struct sycamore_seat *seat);

void seatop_set_basic_full(struct sycamore_seat *seat);

void seatop_set_basic_pointer_no(struct sycamore_seat *seat);

void seatop_set_pointer_down(struct sycamore_seat *seat, struct wlr_surface *surface, double sx, double sy);

void seatop_set_pointer_move(struct sycamore_seat *seat, struct sycamore_view *view);

void seatop_set_pointer_resize(struct sycamore_seat *seat, struct sycamore_view *view, uint32_t edges);

bool seatop_pointer_interactive_mode_check(struct sycamore_seat *seat, struct sycamore_view *view, enum seatop_mode mode);

#endif //SYCAMORE_SEAT_H