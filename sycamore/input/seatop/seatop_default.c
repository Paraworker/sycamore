#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"
#include "sycamore/util/time.h"

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec,
                                   event->button, event->state);

    if (event->state == WLR_BUTTON_PRESSED) {
        /* Focus the view if the button was pressed */
        struct wlr_cursor *cursor = seat->cursor->wlr_cursor;
        view_set_focus(view_under(server.scene, cursor->x, cursor->y));
    }
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    struct sycamore_cursor *cursor = seat->cursor;
    double sx, sy;
    struct wlr_surface *surface = surface_under(server.scene,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);
    pointer_update(cursor, surface, sx, sy, time_msec);
}

static void process_cursor_rebase(struct sycamore_seat *seat) {
    struct sycamore_cursor *cursor = seat->cursor;
    if (!cursor->enabled) {
        return;
    }

    double sx, sy;
    struct wlr_surface *surface = surface_under(server.scene,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);
    pointer_update(cursor, surface, sx, sy, get_current_time_msec());
}

static void process_end(struct sycamore_seat *seat) {}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .cursor_rebase = process_cursor_rebase,
        .end = process_end,
        .mode = SEATOP_DEFAULT,
};

void seatop_begin_default(struct sycamore_seat *seat) {
    seatop_end(seat);

    seat->seatop_impl = &seatop_impl;
    process_cursor_rebase(seat);
}