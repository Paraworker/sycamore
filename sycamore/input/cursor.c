#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/input/cursor.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"
#include "sycamore/util/box.h"

void cursor_set_image(struct sycamore_cursor *cursor, const char *image) {
    if (!cursor->enabled) {
        return;
    }

    if (!image) {
        cursor->image = NULL;
        wlr_cursor_set_image(cursor->wlr_cursor, NULL, 0, 0, 0, 0, 0, 0);
    } else if (!cursor->image || strcmp(cursor->image, image) != 0) {
        cursor->image = image;
        wlr_xcursor_manager_set_cursor_image(cursor->xcursor_manager,
                                             image, cursor->wlr_cursor);
    }
}

void cursor_set_image_surface(struct sycamore_cursor *cursor,
        struct wlr_surface *surface, const int32_t hotspot_x, const int32_t hotspot_y) {
    if (!cursor->enabled) {
        return;
    }

    cursor->image = NULL;
    wlr_cursor_set_surface(cursor->wlr_cursor, surface, hotspot_x, hotspot_y);
}

void cursor_rebase(struct sycamore_cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    seatop_pointer_rebase(cursor->seat);
}

void cursor_enable(struct sycamore_cursor *cursor) {
    if (cursor->enabled) {
        return;
    }

    cursor->enabled = true;
    cursor_rebase(cursor);
}

void cursor_disable(struct sycamore_cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    cursor_set_image(cursor, NULL);
    wlr_seat_pointer_notify_clear_focus(cursor->seat->wlr_seat);
    cursor->enabled = false;
}

bool xcursor_init(struct sycamore_cursor *cursor) {
    /* TODO: Get cursor theme/size from config file
     * recreate xcursor manager if theme/size changed. */
    if (cursor->xcursor_manager) {
        return true;
    }

    unsigned size = 24;
    const char *theme = NULL;

    char size_fmt[16];
    snprintf(size_fmt, sizeof(size_fmt), "%u", size);
    setenv("XCURSOR_SIZE", size_fmt, 1);

    if (theme) {
        setenv("XCURSOR_THEME", theme, 1);
    }

    cursor->xcursor_manager = wlr_xcursor_manager_create(theme, size);
    if (!cursor->xcursor_manager) {
        wlr_log(WLR_ERROR, "Unable to create xcursor manager for theme '%s'", theme);
        return false;
    }

    return true;
}

void cursor_refresh(struct sycamore_cursor *cursor) {
    struct wlr_cursor *cur = cursor->wlr_cursor;
    wlr_cursor_warp(cur, NULL, cur->x, cur->y);

    if (!cursor->enabled) {
        return;
    }

    const char *reset_image = cursor->image;
    if (!reset_image) {
        reset_image = "left_ptr";
    }

    wlr_xcursor_manager_set_cursor_image(cursor->xcursor_manager,
                                         reset_image, cursor->wlr_cursor);
}

void cursor_set_to_output(struct sycamore_cursor *cursor, struct sycamore_output *output) {
    if (!output) {
        return;
    }

    struct wlr_box output_box;
    wlr_output_layout_get_box(server.output_layout, output->wlr_output, &output_box);
    if (wlr_box_empty(&output_box)) {
        wlr_log(WLR_ERROR, "output_box is empty.");
        return;
    }

    int32_t center_x = 0, center_y = 0;
    box_get_center_coords(&output_box, &center_x, &center_y);

    wlr_cursor_warp(cursor->wlr_cursor, NULL, center_x, center_y);

    cursor_set_image(cursor, NULL);
    cursor_rebase(cursor);
}

void xcursor_reload(struct sycamore_cursor *cursor) {
    if (!xcursor_init(cursor)) {
        return;
    }

    struct sycamore_output *output;
    wl_list_for_each(output, &server.all_outputs, link) {
        wlr_xcursor_manager_load(cursor->xcursor_manager,
                                 output->wlr_output->scale);
    }

    cursor_refresh(cursor);
}

struct sycamore_output *cursor_at_output(struct sycamore_cursor *cursor,
        struct wlr_output_layout *layout) {
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;
    struct wlr_output *output = wlr_output_layout_output_at(layout, wlr_cursor->x, wlr_cursor->y);
    if (!output) {
        return NULL;
    }

    return output->data;
}

static void handle_cursor_motion(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
    struct wlr_pointer_motion_event *event = data;

    cursor_enable(cursor);

    wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base,
                    event->delta_x, event->delta_y);
    seatop_pointer_motion(cursor->seat, event->time_msec);
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

    cursor_enable(cursor);

    wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
    seatop_pointer_motion(cursor->seat, event->time_msec);
}

static void handle_cursor_button(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a button event. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
    struct wlr_pointer_button_event *event = data;

    if (event->state == WLR_BUTTON_PRESSED) {
        ++(cursor->pressed_button_count);
    } else if (cursor->pressed_button_count > 0) {
        --(cursor->pressed_button_count);
    }

    cursor_enable(cursor);

    seatop_pointer_button(cursor->seat, event);
}

static void handle_cursor_axis(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
    struct wlr_pointer_axis_event *event = data;

    cursor_enable(cursor);

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

static void handle_swipe_begin(struct wl_listener *listener, void *data) {
    struct sycamore_cursor *cursor = wl_container_of(listener, cursor, swipe_begin);
    struct wlr_pointer_swipe_begin_event *event = data;

    cursor_enable(cursor);

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

    cursor_enable(cursor);

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

    cursor_enable(cursor);

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

    wl_list_remove(&cursor->cursor_motion.link);
    wl_list_remove(&cursor->cursor_motion_absolute.link);
    wl_list_remove(&cursor->cursor_button.link);
    wl_list_remove(&cursor->cursor_axis.link);
    wl_list_remove(&cursor->cursor_frame.link);

    wl_list_remove(&cursor->swipe_begin.link);
    wl_list_remove(&cursor->swipe_update.link);
    wl_list_remove(&cursor->swipe_end.link);
    wl_list_remove(&cursor->pinch_begin.link);
    wl_list_remove(&cursor->pinch_update.link);
    wl_list_remove(&cursor->pinch_end.link);
    wl_list_remove(&cursor->hold_begin.link);
    wl_list_remove(&cursor->hold_end.link);

    if (cursor->xcursor_manager) {
        wlr_xcursor_manager_destroy(cursor->xcursor_manager);
    }

    if (cursor->wlr_cursor) {
        wlr_cursor_destroy(cursor->wlr_cursor);
    }

    free(cursor);
}

struct sycamore_cursor *sycamore_cursor_create(struct sycamore_seat *seat,
        struct wl_display *display, struct wlr_output_layout *output_layout) {
    struct sycamore_cursor *cursor = calloc(1, sizeof(struct sycamore_cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_cursor");
        return NULL;
    }

    cursor->image = NULL;
    cursor->xcursor_manager = NULL;
    cursor->pressed_button_count = 0;
    cursor->enabled = false;
    cursor->seat = seat;

    cursor->wlr_cursor = wlr_cursor_create();
    if (!cursor->wlr_cursor) {
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->wlr_cursor, output_layout);

    if (!xcursor_init(cursor)) {
        wlr_cursor_destroy(cursor->wlr_cursor);
        free(cursor);
        return NULL;
    }

    cursor->gestures = wlr_pointer_gestures_v1_create(display);
    if (!cursor->gestures) {
        wlr_xcursor_manager_destroy(cursor->xcursor_manager);
        wlr_cursor_destroy(cursor->wlr_cursor);
        free(cursor);
        return NULL;
    }

    cursor->cursor_motion.notify = handle_cursor_motion;
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