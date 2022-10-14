#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"
#include "sycamore/util/time.h"

static inline void pointer_update(struct sycamore_cursor *cursor, const uint32_t time_msec) {
    struct wlr_seat *wlr_seat = cursor->seat->wlr_seat;
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;

    double sx, sy;
    struct wlr_surface *surface = find_surface(server.scene, wlr_cursor->x, wlr_cursor->y, &sx, &sy);

    if (surface) {
        wlr_seat_pointer_notify_enter(wlr_seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(wlr_seat, time_msec, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(wlr_seat);
        cursor_set_image(cursor, "left_ptr");
    }
}

static inline void drag_icons_update_position(struct sycamore_seat *seat) {
    struct wlr_scene_node *node;
    wl_list_for_each(node, &server.scene->drag_icons->children, link) {
        seat_drag_icon_update_position(seat, node->data);
    }
}

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec,
                                   event->button, event->state);

    if (event->state == WLR_BUTTON_PRESSED) {
        /* Focus the view if the button was pressed */
        struct wlr_cursor *cursor = seat->cursor->wlr_cursor;
        view_set_focus(find_view(server.scene, cursor->x, cursor->y));
    }
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    pointer_update(seat->cursor, time_msec);
    drag_icons_update_position(seat);
}

static void process_pointer_rebase(struct sycamore_seat *seat) {
    pointer_update(seat->cursor, get_current_time_msec());
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .pointer_rebase = process_pointer_rebase,
        .mode = SEATOP_DEFAULT,
};

void seatop_begin_default(struct sycamore_seat *seat) {
    seatop_end(seat);
    seat->seatop_impl = &seatop_impl;
    cursor_rebase(seat->cursor);
}