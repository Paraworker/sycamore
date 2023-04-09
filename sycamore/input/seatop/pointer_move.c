#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"

static void handlePointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    if (seat->cursor->pressedButtonCount == 0) {
        // If there is no button being pressed
        // we back to default.
        seatopSetDefault(seat);
    }
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    /* Move the grabbed view to the new position. */
    SeatopPointerMoveData *data = &(seat->seatopData.pointerMove);
    struct wlr_cursor *cursor = seat->cursor->wlrCursor;
    View *view = data->viewPtr.view;
    if (!view) {
        return;
    }

    VIEW_MOVE_TO(view, cursor->x - data->dx, cursor->y - data->dy);
}

static void handleEnd(Seat *seat) {
    SeatopPointerMoveData *data = &(seat->seatopData.pointerMove);
    if (data->viewPtr.view) {
        viewPtrDisconnect(&data->viewPtr);
    }
}

static const SeatopImpl impl = {
        .pointerButton = handlePointerButton,
        .pointerMotion = handlePointerMotion,
        .end           = handleEnd,
        .mode          = POINTER_MOVE,
};

void seatopSetPointerMove(Seat *seat, View *view) {
    if (!seatopPointerInteractiveModeCheck(seat, view, POINTER_MOVE)) {
        return;
    }

    seatopEnd(seat);

    SeatopPointerMoveData *data = &(seat->seatopData.pointerMove);

    viewPtrConnect(&data->viewPtr, view);
    data->dx = seat->cursor->wlrCursor->x - VIEW_X(view);
    data->dy = seat->cursor->wlrCursor->y - VIEW_Y(view);

    seat->seatopImpl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlrSeat);
    cursorSetImage(seat->cursor, "grab");
}