#include "sycamore/input/seat.h"

static void handle_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    seatop_set_basic_full(seat);
}

static void handle_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    seatop_set_basic_full(seat);
}

static const struct seatop_impl impl = {
        .pointer_button = handle_pointer_button,
        .pointer_motion = handle_pointer_motion,
        .mode = BASIC_POINTER_NO,
};

void seatop_set_basic_pointer_no(struct sycamore_seat *seat) {
    seatop_end(seat);

    seat->seatop_impl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_hidden(seat->cursor);
}