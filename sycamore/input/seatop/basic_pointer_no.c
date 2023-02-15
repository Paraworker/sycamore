#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"

static void handlePointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    seatopSetBasicFull(seat);
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    seatopSetBasicFull(seat);
}

static const SeatopImpl impl = {
        .pointerButton = handlePointerButton,
        .pointerMotion = handlePointerMotion,
        .mode          = BASIC_POINTER_NO,
};

void seatopSetBasicPointerNo(Seat *seat) {
    seatopEnd(seat);

    seat->seatopImpl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlrSeat);
    cursorSetHidden(seat->cursor);
}