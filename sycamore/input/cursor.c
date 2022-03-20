#include <stdlib.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "sycamore/input/cursor.h"
#include "sycamore/desktop/view.h"

struct wlr_surface* update_pointer_focus(struct sycamore_cursor *cursor,
                                         double* sx, double* sy) {
    struct wlr_surface *surface = NULL;
    struct sycamore_view *view = desktop_view_at(cursor->seat->server,
                                               cursor->wlr_cursor->x, cursor->wlr_cursor->y, &surface, sx, sy);

    if (!view && cursor->setted_to_default == false) {
        /* If there's no view under the cursor, set the cursor image to a
         * default. This is what makes the cursor image to default again
         * when you move out of views.
         *
         * The had_setted_to_default variable can make sure to set image only once. */
        wlr_xcursor_manager_set_cursor_image(
                cursor->xcursor_manager, "left_ptr", cursor->wlr_cursor);

        cursor->setted_to_default = true;
    } else if (view && cursor->setted_to_default == true){
        cursor->setted_to_default = false;
    }

    struct wlr_seat* seat = cursor->seat->wlr_seat;
    if (surface && seat->pointer_state.focused_surface != surface) {
        /*
         * Send pointer enter events.
         *
         * The enter event gives the surface "pointer focus", which is distinct
         * from keyboard focus.
         *
         * Note that wlroots will avoid sending duplicate enter events. */
        wlr_seat_pointer_notify_enter(seat, surface, *sx, *sy);

    } else if (!surface && seat->pointer_state.focused_surface) {
        /* Clear pointer focus so future button events and such are not sent to
         * the last client to have the cursor over it. */
        wlr_seat_pointer_clear_focus(seat);
    }
    return surface;
}

static void process_cursor_move(struct sycamore_cursor *cursor, uint32_t time) {
    /* Move the grabbed view to the new position. */
    struct sycamore_view *view = cursor->grabbed_view;
    if (!view) {
        /* prevent the grabbed view from being unmapped */
        cursor->mode = CURSOR_MODE_PASSTHROUGH;
        return;
    }

    view->x = cursor->wlr_cursor->x - cursor->grab_x;
    view->y = cursor->wlr_cursor->y - cursor->grab_y;

    wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

static void process_cursor_resize(struct sycamore_cursor *cursor, uint32_t time) {
    /*
     * Resizing the grabbed view can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the view
     * on one or two axes, but can also move the view if you resize from the top
     * or left edges (or top-left corner).
     *
     * Note that I took some shortcuts here. In a more fleshed-out compositor,
     * you'd wait for the client to prepare a buffer at the new size, then
     * commit any movement that was prepared.
     */
    struct sycamore_view *view = cursor->grabbed_view;
    if (!view) {
        cursor->mode = CURSOR_MODE_PASSTHROUGH;
        return;
    }

    double border_x = cursor->wlr_cursor->x - cursor->grab_x;
    double border_y = cursor->wlr_cursor->y - cursor->grab_y;
    int new_left = cursor->grab_geobox.x;
    int new_right = cursor->grab_geobox.x + cursor->grab_geobox.width;
    int new_top = cursor->grab_geobox.y;
    int new_bottom = cursor->grab_geobox.y + cursor->grab_geobox.height;

    if (cursor->resize_edges & WLR_EDGE_TOP) {
        new_top = border_y;
        if (new_top >= new_bottom) {
            new_top = new_bottom - 1;
        }
    } else if (cursor->resize_edges & WLR_EDGE_BOTTOM) {
        new_bottom = border_y;
        if (new_bottom <= new_top) {
            new_bottom = new_top + 1;
        }
    }
    if (cursor->resize_edges & WLR_EDGE_LEFT) {
        new_left = border_x;
        if (new_left >= new_right) {
            new_left = new_right - 1;
        }
    } else if (cursor->resize_edges & WLR_EDGE_RIGHT) {
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

static void process_cursor_motion(struct sycamore_cursor *cursor, uint32_t time) {
    if (cursor->mode == CURSOR_MODE_MOVE) {
        process_cursor_move(cursor, time);
        return;
    } else if (cursor->mode == CURSOR_MODE_RESIZE) {
        process_cursor_resize(cursor, time);
        return;
    }

    /* mode is passthrough, update the pointer focus and send the motion event. */
    double sx, sy;
    struct wlr_surface* surface = update_pointer_focus(cursor, &sx, &sy);
    if (surface) {
        wlr_seat_pointer_notify_motion(cursor->seat->wlr_seat, time, sx, sy);
    }
}

static void cursor_motion(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct sycamore_cursor *cursor =
            wl_container_of(listener, cursor, cursor_motion);
    struct wlr_pointer_motion_event *event = data;

    /* move the cursor */
    wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base,
                    event->delta_x, event->delta_y);

    process_cursor_motion(cursor, event->time_msec);
}

static void cursor_motion_absolute(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the window. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    struct sycamore_cursor *cursor =
            wl_container_of(listener, cursor, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
    process_cursor_motion(cursor, event->time_msec);
}

static void cursor_button(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a button
     * event. */
    struct sycamore_cursor *cursor =
            wl_container_of(listener, cursor, cursor_button);
    struct wlr_pointer_button_event *event = data;

    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(cursor->seat->wlr_seat,
                                   event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_PRESSED) {
        /* Focus that client if the button was pressed */
        double sx, sy;
        struct wlr_surface *surface = NULL;
        struct sycamore_view *view = desktop_view_at(cursor->seat->server,
                                                   cursor->wlr_cursor->x, cursor->wlr_cursor->y, &surface, &sx, &sy);
        focus_view(view);
    } else if (cursor->mode != CURSOR_MODE_PASSTHROUGH) {
        /* If you released any buttons and the cursor mode is not passthrough
         * we exit interactive move/resize mode. */
        cursor->mode = CURSOR_MODE_PASSTHROUGH;
        double sx, sy;
        update_pointer_focus(cursor, &sx, &sy);
    }
}

static void cursor_axis(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct sycamore_cursor *cursor =
            wl_container_of(listener, cursor, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(cursor->seat->wlr_seat,
                                 event->time_msec, event->orientation, event->delta,
                                 event->delta_discrete, event->source);
}

static void cursor_frame(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    struct sycamore_cursor *cursor =
            wl_container_of(listener, cursor, cursor_frame);
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(cursor->seat->wlr_seat);
}

void set_interactive(struct sycamore_view *view,
        enum cursor_mode mode, uint32_t edges) {
    /* This function sets up an interactive move or resize operation, where the
     * compositor stops propegating pointer events to clients and instead
     * consumes them itself, to move or resize windows. */
    struct sycamore_server *server = view->server;
    struct wlr_surface *focused_surface =
            server->seat->wlr_seat->pointer_state.focused_surface;

    /* Some clients may emit more than one request_move signal at the same time.
     * If we are already in the move/request mode, do not enter again. */
    if (server->seat->cursor->mode != CURSOR_MODE_PASSTHROUGH) {
        return;
    }

    /* Deny move/resize requests from unfocused clients or there is no focused clients. */
    if (!focused_surface) {
        return;
    }
    if (view->interface->get_wlr_surface(view) !=
        wlr_surface_get_root_surface(focused_surface)) {
        return;
    }

    /* Clear the pointer focus when enter move/resize mode. */
    wlr_seat_pointer_clear_focus(server->seat->wlr_seat);

    server->seat->cursor->grabbed_view = view;
    server->seat->cursor->mode = mode;

    if (mode == CURSOR_MODE_MOVE) {
        wlr_xcursor_manager_set_cursor_image(
                server->seat->cursor->xcursor_manager,
                "grabbing", server->seat->cursor->wlr_cursor);

        server->seat->cursor->grab_x = server->seat->cursor->wlr_cursor->x - view->x;
        server->seat->cursor->grab_y = server->seat->cursor->wlr_cursor->y - view->y;
    } else {
        struct wlr_box geo_box;
        view->interface->get_geometry(view, &geo_box);

        double border_x = (view->x + geo_box.x) +
                          ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
        double border_y = (view->y + geo_box.y) +
                          ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
        server->seat->cursor->grab_x = server->seat->cursor->wlr_cursor->x - border_x;
        server->seat->cursor->grab_y = server->seat->cursor->wlr_cursor->y - border_y;

        server->seat->cursor->grab_geobox = geo_box;
        server->seat->cursor->grab_geobox.x += view->x;
        server->seat->cursor->grab_geobox.y += view->y;

        server->seat->cursor->resize_edges = edges;
    }
}

void sycamore_cursor_destroy(struct sycamore_cursor* cursor) {
    if (!cursor) {
        return;
    }

    if (cursor->xcursor_manager) {
        wlr_xcursor_manager_destroy(cursor->xcursor_manager);
    }
    if (cursor->wlr_cursor) {
        wlr_cursor_destroy(cursor->wlr_cursor);
    }

    wl_list_remove(&cursor->cursor_axis.link);
    wl_list_remove(&cursor->cursor_button.link);
    wl_list_remove(&cursor->cursor_motion.link);
    wl_list_remove(&cursor->cursor_motion_absolute.link);
    wl_list_remove(&cursor->cursor_frame.link);

    free(cursor);
}

struct sycamore_cursor* sycamore_cursor_create(struct sycamore_seat* seat,
        struct wlr_output_layout* output_layout) {
    struct sycamore_cursor* cursor = calloc(1, sizeof(struct sycamore_cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_cursor");
        return NULL;
    }

    cursor->wlr_cursor = wlr_cursor_create();
    if (!cursor->wlr_cursor) {
        sycamore_cursor_destroy(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(
            cursor->wlr_cursor, output_layout);

    cursor->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    if (!cursor->xcursor_manager) {
        sycamore_cursor_destroy(cursor);
        return NULL;
    }

    wlr_xcursor_manager_load(cursor->xcursor_manager, 1);

    /* Set up cursor listeners */
    cursor->cursor_motion.notify = cursor_motion;
    wl_signal_add(&cursor->wlr_cursor->events.motion,
                  &cursor->cursor_motion);
    cursor->cursor_motion_absolute.notify = cursor_motion_absolute;
    wl_signal_add(&cursor->wlr_cursor->events.motion_absolute,
                  &cursor->cursor_motion_absolute);
    cursor->cursor_button.notify = cursor_button;
    wl_signal_add(&cursor->wlr_cursor->events.button,
                  &cursor->cursor_button);
    cursor->cursor_axis.notify = cursor_axis;
    wl_signal_add(&cursor->wlr_cursor->events.axis,
                  &cursor->cursor_axis);
    cursor->cursor_frame.notify = cursor_frame;
    wl_signal_add(&cursor->wlr_cursor->events.frame,
                  &cursor->cursor_frame);

    cursor->seat = seat;
    cursor->mode = CURSOR_MODE_PASSTHROUGH;
    cursor->grabbed_view = NULL;
    cursor->setted_to_default = false;

    return cursor;
}

