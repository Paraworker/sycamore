#include "sycamore/input/seat.h"

static const struct seatop_pointer_impl impl = {
        .mode = BLANK,
};

void seatop_pointer_begin_blank(struct sycamore_seat *seat, const char *cursor_image) {
    seatop_pointer_end(seat);

    seat->seatop_pointer_impl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, cursor_image);
}
