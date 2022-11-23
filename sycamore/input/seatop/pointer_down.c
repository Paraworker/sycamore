#include "sycamore/input/seat.h"

static void handle_surface_destroy(struct wl_listener *listener, void *data) {
    struct seatop_pointer_down_data *d = wl_container_of(listener, d, surface_destroy);
    seatop_set_basic_full(d->seat);
}

static void handle_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec,
                                   event->button, event->state);

    if (seat->cursor->pressed_button_count == 0) {
        seatop_set_basic_full(seat);
    }
}

static void handle_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    struct seatop_pointer_down_data *data = &(seat->seatop_data.pointer_down);

    double sx = data->dx + seat->cursor->wlr_cursor->x;
    double sy = data->dy + seat->cursor->wlr_cursor->y;

    wlr_seat_pointer_notify_motion(seat->wlr_seat, time_msec, sx, sy);
}

static void handle_end(struct sycamore_seat *seat) {
    struct seatop_pointer_down_data *data = &(seat->seatop_data.pointer_down);
    wl_list_remove(&data->surface_destroy.link);
}

static const struct seatop_impl impl = {
        .pointer_button = handle_pointer_button,
        .pointer_motion = handle_pointer_motion,
        .end = handle_end,
        .mode = POINTER_DOWN,
};

void seatop_set_pointer_down(struct sycamore_seat *seat, struct wlr_surface *surface, double sx, double sy) {
    if (!surface) {
        return;
    }

    seatop_end(seat);

    struct seatop_pointer_down_data *data = &(seat->seatop_data.pointer_down);

    data->surface = surface;
    wl_signal_add(&surface->events.destroy, &data->surface_destroy);
    data->surface_destroy.notify = handle_surface_destroy;
    data->dx = sx - seat->cursor->wlr_cursor->x;
    data->dy = sy - seat->cursor->wlr_cursor->y;
    data->seat = seat;

    seat->seatop_impl = &impl;
}