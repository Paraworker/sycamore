#include <wlr/types/wlr_compositor.h>
#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"

static void onSurfaceDestroy(struct wl_listener *listener, void *data) {
    SeatopPointerDownData *d = wl_container_of(listener, d, surfaceDestroy);
    seatopSetDefault(d->seat);
}

static void handlePointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    wlr_seat_pointer_notify_button(seat->wlrSeat, event->time_msec,
                                   event->button, event->state);

    if (seat->cursor->pressedButtonCount == 0) {
        seatopSetDefault(seat);
    }
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    SeatopPointerDownData *data = &(seat->seatopData.pointerDown);

    double sx = data->dx + seat->cursor->wlrCursor->x;
    double sy = data->dy + seat->cursor->wlrCursor->y;

    wlr_seat_pointer_notify_motion(seat->wlrSeat, timeMsec, sx, sy);
}

static void handleEnd(Seat *seat) {
    SeatopPointerDownData *data = &(seat->seatopData.pointerDown);
    wl_list_remove(&data->surfaceDestroy.link);
}

static const SeatopImpl impl = {
        .pointerButton = handlePointerButton,
        .pointerMotion = handlePointerMotion,
        .end           = handleEnd,
        .mode          = POINTER_DOWN,
};

void seatopSetPointerDown(Seat *seat, struct wlr_surface *surface, double sx, double sy) {
    if (!surface) {
        return;
    }

    seatopEnd(seat);

    SeatopPointerDownData *data = &(seat->seatopData.pointerDown);

    data->surface = surface;
    wl_signal_add(&surface->events.destroy, &data->surfaceDestroy);
    data->surfaceDestroy.notify = onSurfaceDestroy;
    data->dx = sx - seat->cursor->wlrCursor->x;
    data->dy = sy - seat->cursor->wlrCursor->y;
    data->seat = seat;

    seat->seatopImpl = &impl;
}