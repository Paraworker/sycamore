#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(seat->wlr_seat,
                                   event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_PRESSED) {
        /* Focus the view if the button was pressed */
        struct sycamore_view *view = view_under(seat->server->scene,
                seat->cursor->wlr_cursor->x, seat->cursor->wlr_cursor->y);

        focus_view(view);
    }
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    double sx, sy;
    struct wlr_surface *surface = surface_under(seat->server->scene,
            seat->cursor->wlr_cursor->x, seat->cursor->wlr_cursor->y, &sx, &sy);

    pointer_focus_update(seat->cursor, surface, sx, sy, time_msec);
    cursor_image_update(seat->cursor, surface);
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .mode = SEATOP_DEFAULT,
};

void seatop_begin_default(struct sycamore_seat* seat) {
    seat->seatop_impl = &seatop_impl;
    cursor_rebase(seat->cursor);
}


