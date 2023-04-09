#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"
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
    viewSetFocus(getViewFromNode(FIND_NODE(cursor->x, cursor->y, &sx, &sy)));

    // Switch to seatopPointerDown if seat has a focused surface.
    seatopSetPointerDown(seat, state->focused_surface, state->sx, state->sy);

    wlr_seat_pointer_notify_button(seat->wlrSeat, event->time_msec, event->button, event->state);
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    seatPointerUpdateFocus(seat, timeMsec);
    dragIconsUpdatePosition(seat);
}

static void handlePointerAxis(Seat *seat, struct wlr_pointer_axis_event *event) {
    wlr_seat_pointer_notify_axis(seat->wlrSeat, event->time_msec, event->orientation,
                                 event->delta, event->delta_discrete, event->source);
}

static void handlePointerSwipeBegin(Seat *seat, struct wlr_pointer_swipe_begin_event *event) {
    wlr_pointer_gestures_v1_send_swipe_begin(seat->cursor->gestures, seat->wlrSeat,
                                             event->time_msec, event->fingers);
}

static void handlePointerSwipeUpdate(Seat *seat, struct wlr_pointer_swipe_update_event *event) {
    wlr_pointer_gestures_v1_send_swipe_update(seat->cursor->gestures, seat->wlrSeat,
                                              event->time_msec, event->dx, event->dy);
}

static void handlePointerSwipeEnd(Seat *seat, struct wlr_pointer_swipe_end_event *event) {
    wlr_pointer_gestures_v1_send_swipe_end(seat->cursor->gestures, seat->wlrSeat,
                                           event->time_msec, event->cancelled);
}

static void handlePointerPinchBegin(Seat *seat, struct wlr_pointer_pinch_begin_event *event) {
    wlr_pointer_gestures_v1_send_pinch_begin(seat->cursor->gestures, seat->wlrSeat,
                                             event->time_msec, event->fingers);
}

static void handlePointerPinchUpdate(Seat *seat, struct wlr_pointer_pinch_update_event *event) {
    wlr_pointer_gestures_v1_send_pinch_update(seat->cursor->gestures, seat->wlrSeat,
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

static void handlePointerPinchEnd(Seat *seat, struct wlr_pointer_pinch_end_event *event) {
    wlr_pointer_gestures_v1_send_pinch_end(seat->cursor->gestures, seat->wlrSeat,
                                           event->time_msec, event->cancelled);
}

static void handlePointerHoldBegin(Seat *seat, struct wlr_pointer_hold_begin_event *event) {
    wlr_pointer_gestures_v1_send_hold_begin(seat->cursor->gestures, seat->wlrSeat,
                                            event->time_msec, event->fingers);
}

static void handlePointerHoldEnd(Seat *seat, struct wlr_pointer_hold_end_event *event) {
    wlr_pointer_gestures_v1_send_hold_end(seat->cursor->gestures, seat->wlrSeat,
                                          event->time_msec, event->cancelled);
}

static void handlePointerRebase(Seat *seat) {
    seatPointerUpdateFocus(seat, getCurrentTimeMsec());
}

static const SeatopImpl impl = {
        .pointerButton      = handlePointerButton,
        .pointerMotion      = handlePointerMotion,
        .pointerAxis        = handlePointerAxis,
        .pointerSwipeBegin  = handlePointerSwipeBegin,
        .pointerSwipeUpdate = handlePointerSwipeUpdate,
        .pointerSwipeEnd    = handlePointerSwipeEnd,
        .pointerPinchBegin  = handlePointerPinchBegin,
        .pointerPinchUpdate = handlePointerPinchUpdate,
        .pointerPinchEnd    = handlePointerPinchEnd,
        .pointerHoldBegin   = handlePointerHoldBegin,
        .pointerHoldEnd     = handlePointerHoldEnd,
        .pointerRebase      = handlePointerRebase,
        .mode               = DEFAULT,
};

void seatopSetDefault(Seat *seat) {
    seatopEnd(seat);
    seat->seatopImpl = &impl;
    cursorRebase(seat->cursor);
}