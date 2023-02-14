#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"
#include "sycamore/util/time.h"

static inline void dragIconsUpdatePosition(Seat *seat) {
    struct wlr_scene_node *node;
    wl_list_for_each(node, &server.scene->dragIcons->children, link) {
        seatDragIconUpdatePosition(seat, node->data);
    }
}

static void handlePointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_RELEASED) {
        wlr_seat_pointer_notify_button(seat->wlrSeat, event->time_msec, event->button, event->state);
        return;
    }

    struct wlr_cursor *cursor = seat->cursor->wlrCursor;
    struct wlr_seat_pointer_state *state = &seat->wlrSeat->pointer_state;

    // If pressed on a view, focus it.
    double sx, sy;
    viewSetFocus(findViewByNode(findNode(server.scene, cursor->x, cursor->y, &sx, &sy)));

    // Switch to seatop_pointer_down if seat has a focused surface.
    seatopSetPointerDown(seat, state->focused_surface, state->sx, state->sy);

    wlr_seat_pointer_notify_button(seat->wlrSeat, event->time_msec, event->button, event->state);
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    seatPointerUpdateFocus(seat, timeMsec);
    dragIconsUpdatePosition(seat);
}

static void handlePointerRebase(Seat *seat) {
    seatPointerUpdateFocus(seat, getCurrentTimeMsec());
}

static const SeatopImpl impl = {
        .pointerButton = handlePointerButton,
        .pointerMotion = handlePointerMotion,
        .pointerRebase = handlePointerRebase,
        .mode          = BASIC_FULL,
};

void seatopSetBasicFull(Seat *seat) {
    seatopEnd(seat);
    seat->seatopImpl = &impl;
    seatPointerUpdateFocus(seat, getCurrentTimeMsec());
}