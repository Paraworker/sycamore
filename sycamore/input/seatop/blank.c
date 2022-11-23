#include "sycamore/input/seat.h"

static const struct seatop_impl impl = {
        .mode = BLANK,
};

void seatop_set_blank(struct sycamore_seat *seat) {
    seatop_end(seat);
    seat->seatop_impl = &impl;
}