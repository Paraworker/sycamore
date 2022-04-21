#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_PRESSED) {
        return;
    }

    /* If you released any buttons
     * switch to default mode. */
    if (seat->grabbed_view) {
        seat->grabbed_view->interface->set_resizing(seat->grabbed_view, false);
    }
    seatop_begin_default(seat);
}

static void process_pointer_motion(struct sycamore_seat *seat, uint32_t time_msec) {
    /* Resizing the grabbed view can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the view
     * on one or two axes, but can also move the view if you resize from the top
     * or left edges (or top-left corner).
     *
     * Note that I took some shortcuts here. In a more fleshed-out compositor,
     * you'd wait for the client to prepare a buffer at the new size, then
     * commit any movement that was prepared. */
    struct sycamore_view *view = seat->grabbed_view;
    if (!view) {
        return;
    }

    double border_x = seat->cursor->wlr_cursor->x - seat->grab_x;
    double border_y = seat->cursor->wlr_cursor->y - seat->grab_y;
    int new_left = seat->grab_geobox.x;
    int new_right = seat->grab_geobox.x + seat->grab_geobox.width;
    int new_top = seat->grab_geobox.y;
    int new_bottom = seat->grab_geobox.y + seat->grab_geobox.height;

    if (seat->resize_edges & WLR_EDGE_TOP) {
        new_top = border_y;
        if (new_top >= new_bottom) {
            new_top = new_bottom - 1;
        }
    } else if (seat->resize_edges & WLR_EDGE_BOTTOM) {
        new_bottom = border_y;
        if (new_bottom <= new_top) {
            new_bottom = new_top + 1;
        }
    }
    if (seat->resize_edges & WLR_EDGE_LEFT) {
        new_left = border_x;
        if (new_left >= new_right) {
            new_left = new_right - 1;
        }
    } else if (seat->resize_edges & WLR_EDGE_RIGHT) {
        new_right = border_x;
        if (new_right <= new_left) {
            new_right = new_left + 1;
        }
    }

    struct wlr_box geo_box;
    view->interface->get_geometry(view, &geo_box);

    view->x = new_left - geo_box.x;
    view->y = new_top - geo_box.y;

    int new_width = new_right - new_left;
    int new_height = new_bottom - new_top;

    wlr_scene_node_set_position(view->scene_node, view->x, view->y);
    view->interface->set_size(view, new_width, new_height);
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .mode = SEATOP_POINTER_RESIZE,
};

void seatop_begin_pointer_resize(struct sycamore_seat* seat, struct sycamore_view *view, uint32_t edges) {
    /* Deny resize if we are already in the resize mode. */
    if (seat->seatop_impl->mode == SEATOP_POINTER_RESIZE) {
        return;
    }

    /* Deny resize from maximized/fullscreen clients. */
    if (view->is_maximized || view->is_fullscreen) {
        return;
    }

    /* Deny resize from unfocused clients or there is no focused clients. */
    if (view != seat->server->desktop_focused_view) {
        return;
    }

    seat->seatop_impl = &seatop_impl;

    struct wlr_box geo_box;
    view->interface->get_geometry(view, &geo_box);

    double border_x = (view->x + geo_box.x) +
                      ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    seat->grab_x = seat->cursor->wlr_cursor->x - border_x;
    seat->grab_y = seat->cursor->wlr_cursor->y - border_y;

    seat->grab_geobox = geo_box;
    seat->grab_geobox.x += view->x;
    seat->grab_geobox.y += view->y;

    seat->resize_edges = edges;
    seat->grabbed_view = view;

    view->interface->set_resizing(view, true);

    const char *image = wlr_xcursor_get_resize_name(edges);
    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, image);
}
