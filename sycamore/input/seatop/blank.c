#include "sycamore/input/seat.h"

static const SeatopImpl impl = {
        .mode = BLANK,
};

void seatopSetBlank(Seat *seat) {
    seatopEnd(seat);
    seat->seatopImpl = &impl;
}