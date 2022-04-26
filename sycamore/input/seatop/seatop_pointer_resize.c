#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"

struct pointer_resize_data {
    struct view_ptr view_ptr;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t edges;
};

static void process_pointer_button(struct sycamore_seat *seat, struct wlr_pointer_button_event *event) {
    if (event->state == WLR_BUTTON_PRESSED) {
        return;
    }

    /* If you released any buttons
     * switch to default mode. */
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
    struct pointer_resize_data *data = seat->seatop_data;
    struct sycamore_view *view = data->view_ptr.view;
    if (!view) {
        return;
    }

    double border_x = seat->cursor->wlr_cursor->x - data->grab_x;
    double border_y = seat->cursor->wlr_cursor->y - data->grab_y;
    int new_left = data->grab_geobox.x;
    int new_right = data->grab_geobox.x + data->grab_geobox.width;
    int new_top = data->grab_geobox.y;
    int new_bottom = data->grab_geobox.y + data->grab_geobox.height;

    if (data->edges & WLR_EDGE_TOP) {
        new_top = border_y;
        if (new_top >= new_bottom) {
            new_top = new_bottom - 1;
        }
    } else if (data->edges & WLR_EDGE_BOTTOM) {
        new_bottom = border_y;
        if (new_bottom <= new_top) {
            new_bottom = new_top + 1;
        }
    }
    if (data->edges & WLR_EDGE_LEFT) {
        new_left = border_x;
        if (new_left >= new_right) {
            new_left = new_right - 1;
        }
    } else if (data->edges & WLR_EDGE_RIGHT) {
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

static void process_cursor_rebase(struct sycamore_seat *seat) {
    if (!seat->cursor->enabled) {
        return;
    }

    struct pointer_resize_data *data = seat->seatop_data;
    const char *image = wlr_xcursor_get_resize_name(data->edges);

    wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    cursor_set_image(seat->cursor, image);
}

static void process_end(struct sycamore_seat *seat) {
    struct pointer_resize_data *data = seat->seatop_data;
    if (data->view_ptr.view) {
        data->view_ptr.view->interface->set_resizing(data->view_ptr.view, false);
        view_ptr_disconnect(&data->view_ptr);
    }
}

static const struct sycamore_seatop_impl seatop_impl = {
        .pointer_button = process_pointer_button,
        .pointer_motion = process_pointer_motion,
        .cursor_rebase = process_cursor_rebase,
        .end = process_end,
        .mode = SEATOP_POINTER_RESIZE,
};

void seatop_begin_pointer_resize(struct sycamore_seat* seat, struct sycamore_view *view, uint32_t edges) {
    if (!seatop_interactive_assert(seat, view)) {
        return;
    }

    seatop_end(seat);

    struct pointer_resize_data *data = calloc(1, sizeof(struct pointer_resize_data));
    if (!data) {
        wlr_log(WLR_ERROR, "Unable to allocate pointer_resize_data");
        return;
    }

    view_ptr_connect(&data->view_ptr, view);

    struct wlr_box geo_box;
    view->interface->get_geometry(view, &geo_box);

    double border_x = (view->x + geo_box.x) +
                      ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    data->grab_x = seat->cursor->wlr_cursor->x - border_x;
    data->grab_y = seat->cursor->wlr_cursor->y - border_y;

    data->grab_geobox = geo_box;
    data->grab_geobox.x += view->x;
    data->grab_geobox.y += view->y;

    data->edges = edges;

    seat->seatop_impl = &seatop_impl;
    seat->seatop_data = data;

    view->interface->set_resizing(view, true);
    process_cursor_rebase(seat);
}
