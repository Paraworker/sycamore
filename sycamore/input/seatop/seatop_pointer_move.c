#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_PRESSED) {
        return;
    }

    /* If you released any buttons
     * switch to seatop_default. */
    seatop_begin_default(seat);
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    /* Move the grabbed view to the new position. */
    struct sycamore_view *view = seat->grabbed_view;
    if (!view) {
        return;
    }

    view->x = seat->cursor->wlr_cursor->x - seat->grab_x;
    view->y = seat->cursor->wlr_cursor->y - seat->grab_y;

    wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

static void process_cursor_rebase(struct sycamore_seat *seat) {
    if (!seat->cursor->enabled) {
        return;
    }

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, "grabbing");
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .cursor_rebase = process_cursor_rebase,
        .mode = SEATOP_POINTER_MOVE,
};

void seatop_begin_pointer_move(struct sycamore_seat* seat, struct sycamore_view *view) {
    /* Deny move if we are already in the move mode. */
    if (seat->seatop_impl->mode == SEATOP_POINTER_MOVE) {
        return;
    }

    /* Deny move from maximized/fullscreen clients. */
    if (view->is_maximized || view->is_fullscreen) {
        return;
    }

    /* Deny move from unfocused clients or there is no focused clients. */
    if (view != seat->server->desktop_focused_view) {
        return;
    }

    seat->seatop_impl = &seatop_impl;

    seat->grab_x = seat->cursor->wlr_cursor->x - view->x;
    seat->grab_y = seat->cursor->wlr_cursor->y - view->y;

    seat->grabbed_view = view;

    process_cursor_rebase(seat);
}
