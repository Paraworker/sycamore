#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/desktop/scene.h"
#include "sycamore/input/cursor.h"
#include "sycamore/output/output.h"

static uint32_t get_current_time_msec() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

void cursor_image_update(struct sycamore_cursor *cursor, struct wlr_surface *surface) {
    if (!surface && cursor->set_image_default == false) {
        /* If there's no surface under the cursor, set the cursor image to a default */
        cursor_set_image(cursor, "left_ptr");
        cursor->set_image_default = true;
    } else if (surface && cursor->set_image_default == true){
        cursor->set_image_default = false;
    }
}

void pointer_focus_update(struct sycamore_cursor *cursor, struct wlr_surface *surface, double sx, double sy, uint32_t time_msec) {
    struct wlr_seat* seat = cursor->seat->wlr_seat;
    if (surface) {
        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(cursor->seat->wlr_seat, time_msec, sx, sy);
    } else if (seat->pointer_state.focused_surface) {
        wlr_seat_pointer_clear_focus(seat);
    }
}

void cursor_set_image(struct sycamore_cursor *cursor, const char *name) {
    wlr_xcursor_manager_set_cursor_image(cursor->xcursor_manager,
                                         name, cursor->wlr_cursor);
}

void cursor_warp_to_output(struct sycamore_cursor *cursor, struct sycamore_output *output) {
    struct wlr_box box;
    wlr_output_layout_get_box(cursor->seat->server->output_layout,
                              output->wlr_output, &box);

    cursor->wlr_cursor->x = box.width/2 + box.x;
    cursor->wlr_cursor->y = box.height/2 + box.y;

    wlr_cursor_warp(cursor->wlr_cursor, NULL,
                    cursor->wlr_cursor->x, cursor->wlr_cursor->y);
}

void cursor_enable(struct sycamore_cursor *cursor, bool enabled) {
    if (cursor->enabled == enabled) {
        return;
    }

    cursor->enabled = enabled;
    if (enabled) {
        cursor_rebase(cursor);
    } else {
        wlr_seat_pointer_notify_clear_focus(cursor->seat->wlr_seat);
        wlr_cursor_set_image(cursor->wlr_cursor, NULL, 0, 0, 0, 0, 0, 0);
    }
}

void cursor_rebase(struct sycamore_cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    double sx, sy;
    struct wlr_surface *surface = surface_under(cursor->seat->server->scene,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);

    pointer_focus_update(cursor, surface, sx, sy,
                         get_current_time_msec());
    cursor_image_update(cursor, surface);
}

static void process_pointer_move(struct sycamore_cursor *cursor, uint32_t time) {
    /* Move the grabbed view to the new position. */
    struct sycamore_view *view = cursor->grabbed_view;
    view->x = cursor->wlr_cursor->x - cursor->grab_x;
    view->y = cursor->wlr_cursor->y - cursor->grab_y;

    wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

static void process_pointer_resize(struct sycamore_cursor *cursor, uint32_t time) {
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

static void process_pointer_motion(struct sycamore_cursor *cursor, uint32_t time) {
    if (cursor->mode == CURSOR_MODE_MOVE) {
        process_pointer_move(cursor, time);
        return;
    } else if (cursor->mode == CURSOR_MODE_RESIZE) {
        process_pointer_resize(cursor, time);
        return;
    }

    /* mode is passthrough */
    double sx, sy;
    struct wlr_surface *surface = surface_under(cursor->seat->server->scene,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);

    pointer_focus_update(cursor, surface, sx, sy, time);
    cursor_image_update(cursor, surface);
}

static void handle_cursor_motion_relative(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    cursor_enable(cursor, true);
    wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base,
                    event->delta_x, event->delta_y);
    process_pointer_motion(cursor, event->time_msec);
}

static void handle_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the window. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    cursor_enable(cursor, true);
    wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
    process_pointer_motion(cursor, event->time_msec);
}

static void handle_cursor_button(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a button event. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
    struct wlr_pointer_button_event *event = data;
    cursor_enable(cursor, true);

    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(cursor->seat->wlr_seat,
                                   event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_PRESSED) {
        /* Focus the view if the button was pressed */
        struct sycamore_view *view = view_under(cursor->seat->server->scene,
                                                cursor->wlr_cursor->x, cursor->wlr_cursor->y);

        focus_view(view);
    } else if (cursor->mode != CURSOR_MODE_PASSTHROUGH) {
        /* If you released any buttons and the cursor mode is not passthrough
         * exit interactive move/resize mode. */
        cursor->mode = CURSOR_MODE_PASSTHROUGH;
        cursor_rebase(cursor);
    }
}

static void handle_cursor_axis(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    cursor_enable(cursor, true);
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(cursor->seat->wlr_seat,
                                 event->time_msec, event->orientation, event->delta,
                                 event->delta_discrete, event->source);
}

static void handle_cursor_frame(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_frame);
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

    /* Deny move/resize requests from maximized/fullscreen clients. */
    if (view->is_maximized || view->is_fullscreen) {
        return;
    }

    /* Deny move/resize requests from unfocused clients or there is no focused clients. */
    if (focused_surface == NULL || view->interface->get_wlr_surface(view) !=
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

static void handle_swipe_begin(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, swipe_begin);
    struct wlr_pointer_swipe_begin_event *event = data;
    cursor_enable(cursor, true);
    wlr_pointer_gestures_v1_send_swipe_begin(cursor->gestures, cursor->seat->wlr_seat,
                                             event->time_msec, event->fingers);
}

static void handle_swipe_update(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, swipe_update);
    struct wlr_pointer_swipe_update_event *event = data;

    wlr_pointer_gestures_v1_send_swipe_update(cursor->gestures, cursor->seat->wlr_seat,
                                              event->time_msec, event->dx, event->dy);
}

static void handle_swipe_end(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, swipe_end);
    struct wlr_pointer_swipe_end_event *event = data;

    wlr_pointer_gestures_v1_send_swipe_end(cursor->gestures, cursor->seat->wlr_seat,
                                           event->time_msec, event->cancelled);
}

static void handle_pinch_begin(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, pinch_begin);
    struct wlr_pointer_pinch_begin_event *event = data;
    cursor_enable(cursor, true);
    wlr_pointer_gestures_v1_send_pinch_begin(cursor->gestures, cursor->seat->wlr_seat,
                                             event->time_msec, event->fingers);
}

static void handle_pinch_update(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, pinch_update);
    struct wlr_pointer_pinch_update_event *event = data;

    wlr_pointer_gestures_v1_send_pinch_update(cursor->gestures, cursor->seat->wlr_seat,
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

static void handle_pinch_end(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, pinch_end);
    struct wlr_pointer_pinch_end_event *event = data;

    wlr_pointer_gestures_v1_send_pinch_end(cursor->gestures, cursor->seat->wlr_seat,
                                           event->time_msec, event->cancelled);
}

static void handle_hold_begin(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, hold_begin);
    struct wlr_pointer_hold_begin_event *event = data;
    cursor_enable(cursor, true);
    wlr_pointer_gestures_v1_send_hold_begin(cursor->gestures, cursor->seat->wlr_seat,
                                            event->time_msec, event->fingers);
}

static void handle_hold_end(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, hold_end);
    struct wlr_pointer_hold_end_event *event = data;

    wlr_pointer_gestures_v1_send_hold_end(cursor->gestures, cursor->seat->wlr_seat,
                                          event->time_msec, event->cancelled);
}

void sycamore_cursor_destroy(struct sycamore_cursor *cursor) {
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

    wl_list_remove(&cursor->swipe_begin.link);
    wl_list_remove(&cursor->swipe_update.link);
    wl_list_remove(&cursor->swipe_end.link);
    wl_list_remove(&cursor->pinch_begin.link);
    wl_list_remove(&cursor->pinch_update.link);
    wl_list_remove(&cursor->hold_begin.link);
    wl_list_remove(&cursor->hold_end.link);

    free(cursor);
}

struct sycamore_cursor *sycamore_cursor_create(struct sycamore_seat *seat,
        struct wlr_output_layout *output_layout) {
    struct sycamore_cursor *cursor = calloc(1, sizeof(struct sycamore_cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_cursor");
        return NULL;
    }

    cursor->wlr_cursor = wlr_cursor_create();
    if (!cursor->wlr_cursor) {
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->wlr_cursor, output_layout);

    cursor->gestures = wlr_pointer_gestures_v1_create(seat->server->wl_display);
    if (!cursor->gestures) {
        sycamore_cursor_destroy(cursor);
        return NULL;
    }

    cursor->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    if (!cursor->xcursor_manager) {
        sycamore_cursor_destroy(cursor);
        return NULL;
    }

    wlr_xcursor_manager_load(cursor->xcursor_manager, 1);

    cursor->mode = CURSOR_MODE_PASSTHROUGH;
    cursor->enabled = false;
    cursor->set_image_default = false;
    cursor->grabbed_view = NULL;
    cursor->seat = seat;

    cursor->cursor_motion.notify = handle_cursor_motion_relative;
    wl_signal_add(&cursor->wlr_cursor->events.motion, &cursor->cursor_motion);
    cursor->cursor_motion_absolute.notify = handle_cursor_motion_absolute;
    wl_signal_add(&cursor->wlr_cursor->events.motion_absolute, &cursor->cursor_motion_absolute);
    cursor->cursor_button.notify = handle_cursor_button;
    wl_signal_add(&cursor->wlr_cursor->events.button, &cursor->cursor_button);
    cursor->cursor_axis.notify = handle_cursor_axis;
    wl_signal_add(&cursor->wlr_cursor->events.axis, &cursor->cursor_axis);
    cursor->cursor_frame.notify = handle_cursor_frame;
    wl_signal_add(&cursor->wlr_cursor->events.frame, &cursor->cursor_frame);

    cursor->swipe_begin.notify = handle_swipe_begin;
    wl_signal_add(&cursor->wlr_cursor->events.swipe_begin, &cursor->swipe_begin);
    cursor->swipe_update.notify = handle_swipe_update;
    wl_signal_add(&cursor->wlr_cursor->events.swipe_update, &cursor->swipe_update);
    cursor->swipe_end.notify = handle_swipe_end;
    wl_signal_add(&cursor->wlr_cursor->events.swipe_end, &cursor->swipe_end);
    cursor->pinch_begin.notify = handle_pinch_begin;
    wl_signal_add(&cursor->wlr_cursor->events.pinch_begin, &cursor->pinch_begin);
    cursor->pinch_update.notify = handle_pinch_update;
    wl_signal_add(&cursor->wlr_cursor->events.pinch_update, &cursor->pinch_update);
    cursor->pinch_end.notify = handle_pinch_end;
    wl_signal_add(&cursor->wlr_cursor->events.pinch_end, &cursor->pinch_end);
    cursor->hold_begin.notify = handle_hold_begin;
    wl_signal_add(&cursor->wlr_cursor->events.hold_begin, &cursor->hold_begin);
    cursor->hold_end.notify = handle_hold_end;
    wl_signal_add(&cursor->wlr_cursor->events.hold_end, &cursor->hold_end);

    return cursor;
}

