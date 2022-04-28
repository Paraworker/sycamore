#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

struct seatop_pointer_move_data {
    struct view_ptr view_ptr;
    double dx, dy;
};

static void process_pointer_button(struct sycamore_seat *seat,
        struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_PRESSED) {
        return;
    }

    /* If you released any buttons
     * switch to seatop_default. */
    seatop_begin_default(seat);
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    /* Move the grabbed view to the new position. */
    struct seatop_pointer_move_data *data = seat->seatop_data;
    struct wlr_cursor *cursor = seat->cursor->wlr_cursor;
    struct sycamore_view *view = data->view_ptr.view;
    if (!view) {
        return;
    }

    view->x = cursor->x - data->dx;
    view->y = cursor->y - data->dy;

    wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

static void process_cursor_rebase(struct sycamore_seat *seat) {
    if (!seat->cursor->enabled) {
        return;
    }

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, "grabbing");
}

static void process_end(struct sycamore_seat *seat) {
    struct seatop_pointer_move_data *data = seat->seatop_data;
    if (data->view_ptr.view) {
        view_ptr_disconnect(&data->view_ptr);
    }
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .cursor_rebase = process_cursor_rebase,
        .end = process_end,
        .mode = SEATOP_POINTER_MOVE,
};

void seatop_begin_pointer_move(struct sycamore_seat *seat, struct sycamore_view *view) {
    if (!seatop_interactive_assert(seat, view)) {
        return;
    }

    seatop_end(seat);

    struct seatop_pointer_move_data *data =
            calloc(1, sizeof(struct seatop_pointer_move_data));
    if (!data) {
        wlr_log(WLR_ERROR, "Unable to allocate seatop_pointer_move_data");
        return;
    }

    view_ptr_connect(&data->view_ptr, view);
    data->dx = seat->cursor->wlr_cursor->x - view->x;
    data->dy = seat->cursor->wlr_cursor->y - view->y;

    seat->seatop_impl = &seatop_impl;
    seat->seatop_data = data;

    process_cursor_rebase(seat);
}
