#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

static void process_button(struct sycamore_seat *seat,
        struct wlr_pointer_button_event *event) {
    if (seat->cursor->pressed_button_count == 0) {
        // If there is no button being pressed
        // we back to default.
        seatop_pointer_begin_full(seat);
    }
}

static void process_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    /* Move the grabbed view to the new position. */
    struct seatop_pointer_move_data *data = &(seat->seatop_pointer_data.move);
    struct wlr_cursor *cursor = seat->cursor->wlr_cursor;
    struct sycamore_view *view = data->view_ptr.view;
    if (!view) {
        return;
    }

    view_move_to(view, cursor->x - data->dx, cursor->y - data->dy);
}

static void process_end(struct sycamore_seat *seat) {
    struct seatop_pointer_move_data *data = &(seat->seatop_pointer_data.move);
    if (data->view_ptr.view) {
        view_ptr_disconnect(&data->view_ptr);
    }
}

static const struct seatop_pointer_impl impl = {
        .button = process_button,
        .motion = process_motion,
        .end = process_end,
        .mode = MOVE,
};

void seatop_pointer_begin_move(struct sycamore_seat *seat, struct sycamore_view *view) {
    if (!seatop_pointer_interactive_check(seat, view, MOVE)) {
        return;
    }

    seatop_pointer_end(seat);

    struct seatop_pointer_move_data *data = &(seat->seatop_pointer_data.move);

    view_ptr_connect(&data->view_ptr, view);
    data->dx = seat->cursor->wlr_cursor->x - view->x;
    data->dy = seat->cursor->wlr_cursor->y - view->y;

    seat->seatop_pointer_impl = &impl;

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, "grabbing");
}