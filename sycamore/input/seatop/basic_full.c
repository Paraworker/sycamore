#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"
#include "sycamore/util/time.h"

static inline void pointer_update_focus(struct sycamore_cursor *cursor, const uint32_t time_msec) {
    struct wlr_seat *wlr_seat = cursor->seat->wlr_seat;
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;

    double sx, sy;
    struct wlr_surface *surface = find_surface_by_node(find_node(server.scene, wlr_cursor->x, wlr_cursor->y, &sx, &sy));

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

static void handle_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_RELEASED) {
        wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec, event->button, event->state);
        return;
    }

    struct wlr_cursor *cursor = seat->cursor->wlr_cursor;
    struct wlr_seat_pointer_state *state = &seat->wlr_seat->pointer_state;

    // If pressed on a view, focus it.
    double sx, sy;
    view_set_focus(find_view_by_node(find_node(server.scene, cursor->x, cursor->y, &sx, &sy)));

    // Switch to seatop_pointer_down if seat has a focused surface.
    seatop_set_pointer_down(seat, state->focused_surface, state->sx, state->sy);

    wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec, event->button, event->state);
}

static void handle_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    pointer_update_focus(seat->cursor, time_msec);
    drag_icons_update_position(seat);
}

static void handle_pointer_rebase(struct sycamore_seat *seat) {
    pointer_update_focus(seat->cursor, get_current_time_msec());
}

static const struct seatop_impl impl = {
        .pointer_button = handle_pointer_button,
        .pointer_motion = handle_pointer_motion,
        .pointer_rebase = handle_pointer_rebase,
        .mode = BASIC_FULL,
};

void seatop_set_basic_full(struct sycamore_seat *seat) {
    seatop_end(seat);
    seat->seatop_impl = &impl;
    pointer_update_focus(seat->cursor, get_current_time_msec());
}