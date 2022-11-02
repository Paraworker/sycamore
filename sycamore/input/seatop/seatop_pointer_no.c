#include "sycamore/input/seat.h"

static void process_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    struct seatop_pointer_no_data *data = &(seat->seatop_pointer_data.no);

    switch (data->end_mode) {
        case BLANK:
            seatop_pointer_begin_blank(seat, "left_ptr");
            break;
        case FULL:
            seatop_pointer_begin_full(seat);
            break;
        default:
            break;
    }
}

static void process_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    struct seatop_pointer_no_data *data = &(seat->seatop_pointer_data.no);

    switch (data->end_mode) {
        case BLANK:
            seatop_pointer_begin_blank(seat, "left_ptr");
            break;
        case FULL:
            seatop_pointer_begin_full(seat);
            break;
        default:
            break;
    }
}

static const struct seatop_pointer_impl impl = {
        .button = process_button,
        .motion = process_motion,
        .mode = NO,
};

void seatop_pointer_begin_no(struct sycamore_seat *seat, enum seatop_pointer_mode end_mode) {
    seatop_pointer_end(seat);

    struct seatop_pointer_no_data *data = &(seat->seatop_pointer_data.no);
    data->end_mode = end_mode;

    seat->seatop_pointer_impl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_hidden(seat->cursor);
}