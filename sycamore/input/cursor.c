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
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"

static bool xcursorInit(Cursor *cursor, const char *theme, uint32_t size) {
    if (cursor->xcursorManager) {
        wlr_xcursor_manager_destroy(cursor->xcursorManager);
        cursor->xcursorManager = NULL;
    }

    char sizeFmt[16];
    snprintf(sizeFmt, sizeof(sizeFmt), "%u", size);
    setenv("XCURSOR_SIZE", sizeFmt, 1);

    if (theme) {
        setenv("XCURSOR_THEME", theme, 1);
    }

    cursor->xcursorManager = wlr_xcursor_manager_create(theme, size);
    if (!cursor->xcursorManager) {
        wlr_log(WLR_ERROR, "Unable to create xcursor manager for theme '%s'", theme);
        return false;
    }

    return true;
}

void cursorSetHidden(Cursor *cursor) {
    cursor->imageMode = HIDDEN;
    cursor->image     = NULL;
    wlr_cursor_set_image(cursor->wlrCursor, NULL, 0, 0, 0, 0, 0, 0);
}

void cursorSetImage(Cursor *cursor, const char *image) {
    if (!image) {
        cursorSetHidden(cursor);
        return;
    }

    if (cursor->image && strcmp(cursor->image, image) == 0) {
        return;
    }

    cursor->imageMode = IMAGE;
    cursor->image     = image;
    wlr_xcursor_manager_set_cursor_image(cursor->xcursorManager,
                                         image, cursor->wlrCursor);
}

void cursorSetImageSurface(Cursor *cursor, struct wlr_surface *surface,
                           int32_t hotspotX, int32_t hotspotY) {
    if (!surface) {
        cursorSetHidden(cursor);
        return;
    }

    cursor->imageMode = SURFACE;
    cursor->image     = NULL;
    wlr_cursor_set_surface(cursor->wlrCursor, surface, hotspotX, hotspotY);
}

void cursorWarp(Cursor *cursor, double lx, double ly) {
    wlr_cursor_warp(cursor->wlrCursor, NULL, lx, ly);
}

void cursorRefresh(Cursor *cursor) {
    cursorWarp(cursor, cursor->wlrCursor->x, cursor->wlrCursor->y);

    if (cursor->imageMode == HIDDEN) {
        return;
    }

    cursor->imageMode = IMAGE;
    if (!cursor->image) {
        cursor->image = "default";
    }

    wlr_xcursor_manager_set_cursor_image(cursor->xcursorManager,
                                         cursor->image, cursor->wlrCursor);
}

void xcursorReload(Cursor *cursor, const char *theme, uint32_t size) {
    if (!xcursorInit(cursor, theme, size)) {
        return;
    }

    Output *output;
    wl_list_for_each(output, &server.allOutputs, link) {
        wlr_xcursor_manager_load(cursor->xcursorManager, output->wlrOutput->scale);
    }

    cursorRefresh(cursor);
}

Output *cursorAtOutput(Cursor *cursor, struct wlr_output_layout *layout) {
    struct wlr_cursor *wlr_cursor = cursor->wlrCursor;
    struct wlr_output *output = wlr_output_layout_output_at(layout, wlr_cursor->x, wlr_cursor->y);
    if (!output) {
        return NULL;
    }

    return output->data;
}

static void onCursorMotion(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    Cursor *cursor = wl_container_of(listener, cursor, cursorMotion);
    struct wlr_pointer_motion_event *event = data;

    wlr_cursor_move(cursor->wlrCursor, &event->pointer->base,
                    event->delta_x, event->delta_y);
    seatopPointerMotion(cursor->seat, event->time_msec);
}

static void onCursorMotionAbsolute(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the window. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    Cursor *cursor = wl_container_of(listener, cursor, cursorMotionAbsolute);
    struct wlr_pointer_motion_absolute_event *event = data;

    wlr_cursor_warp_absolute(cursor->wlrCursor, &event->pointer->base, event->x, event->y);
    seatopPointerMotion(cursor->seat, event->time_msec);
}

static void onCursorButton(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a button event. */
    Cursor *cursor = wl_container_of(listener, cursor, cursorButton);
    struct wlr_pointer_button_event *event = data;

    if (event->state == WLR_BUTTON_PRESSED) {
        ++(cursor->pressedButtonCount);
    } else if (cursor->pressedButtonCount > 0) {
        --(cursor->pressedButtonCount);
    }

    seatopPointerButton(cursor->seat, event);
}

static void onCursorAxis(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    Cursor *cursor = wl_container_of(listener, cursor, cursorAxis);
    struct wlr_pointer_axis_event *event = data;

    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(cursor->seat->wlrSeat,
                                 event->time_msec, event->orientation, event->delta,
                                 event->delta_discrete, event->source);
}

static void onCursorFrame(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    Cursor *cursor = wl_container_of(listener, cursor, cursorFrame);
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(cursor->seat->wlrSeat);
}

static void onSwipeBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeBegin);
    struct wlr_pointer_swipe_begin_event *event = data;

    wlr_pointer_gestures_v1_send_swipe_begin(cursor->gestures, cursor->seat->wlrSeat,
                                             event->time_msec, event->fingers);
}

static void onSwipeUpdate(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeUpdate);
    struct wlr_pointer_swipe_update_event *event = data;

    wlr_pointer_gestures_v1_send_swipe_update(cursor->gestures, cursor->seat->wlrSeat,
                                              event->time_msec, event->dx, event->dy);
}

static void onSwipeEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeEnd);
    struct wlr_pointer_swipe_end_event *event = data;

    wlr_pointer_gestures_v1_send_swipe_end(cursor->gestures, cursor->seat->wlrSeat,
                                           event->time_msec, event->cancelled);
}

static void onPinchBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchBegin);
    struct wlr_pointer_pinch_begin_event *event = data;

    wlr_pointer_gestures_v1_send_pinch_begin(cursor->gestures, cursor->seat->wlrSeat,
                                             event->time_msec, event->fingers);
}

static void onPinchUpdate(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchUpdate);
    struct wlr_pointer_pinch_update_event *event = data;

    wlr_pointer_gestures_v1_send_pinch_update(cursor->gestures, cursor->seat->wlrSeat,
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

static void onPinchEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchEnd);
    struct wlr_pointer_pinch_end_event *event = data;

    wlr_pointer_gestures_v1_send_pinch_end(cursor->gestures, cursor->seat->wlrSeat,
                                           event->time_msec, event->cancelled);
}

static void onHoldBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, holdBegin);
    struct wlr_pointer_hold_begin_event *event = data;

    wlr_pointer_gestures_v1_send_hold_begin(cursor->gestures, cursor->seat->wlrSeat,
                                            event->time_msec, event->fingers);
}

static void onHoldEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, holdEnd);
    struct wlr_pointer_hold_end_event *event = data;

    wlr_pointer_gestures_v1_send_hold_end(cursor->gestures, cursor->seat->wlrSeat,
                                          event->time_msec, event->cancelled);
}

void cursorDestroy(Cursor *cursor) {
    if (!cursor) {
        return;
    }

    wl_list_remove(&cursor->cursorMotion.link);
    wl_list_remove(&cursor->cursorMotionAbsolute.link);
    wl_list_remove(&cursor->cursorButton.link);
    wl_list_remove(&cursor->cursorAxis.link);
    wl_list_remove(&cursor->cursorFrame.link);

    wl_list_remove(&cursor->swipeBegin.link);
    wl_list_remove(&cursor->swipeUpdate.link);
    wl_list_remove(&cursor->swipeEnd.link);
    wl_list_remove(&cursor->pinchBegin.link);
    wl_list_remove(&cursor->pinchUpdate.link);
    wl_list_remove(&cursor->pinchEnd.link);
    wl_list_remove(&cursor->holdBegin.link);
    wl_list_remove(&cursor->holdEnd.link);

    if (cursor->xcursorManager) {
        wlr_xcursor_manager_destroy(cursor->xcursorManager);
    }

    if (cursor->wlrCursor) {
        wlr_cursor_destroy(cursor->wlrCursor);
    }

    free(cursor);
}

Cursor *cursorCreate(Seat *seat, struct wl_display *display, struct wlr_output_layout *layout) {
    Cursor *cursor = calloc(1, sizeof(Cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Unable to allocate Cursor");
        return NULL;
    }

    cursor->imageMode          = HIDDEN;
    cursor->image              = NULL;
    cursor->xcursorManager     = NULL;
    cursor->pressedButtonCount = 0;
    cursor->seat               = seat;

    cursor->wlrCursor = wlr_cursor_create();
    if (!cursor->wlrCursor) {
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->wlrCursor, layout);

    if (!xcursorInit(cursor, NULL, 24)) {
        wlr_cursor_destroy(cursor->wlrCursor);
        free(cursor);
        return NULL;
    }

    cursor->gestures = wlr_pointer_gestures_v1_create(display);
    if (!cursor->gestures) {
        wlr_xcursor_manager_destroy(cursor->xcursorManager);
        wlr_cursor_destroy(cursor->wlrCursor);
        free(cursor);
        return NULL;
    }

    cursor->cursorMotion.notify = onCursorMotion;
    wl_signal_add(&cursor->wlrCursor->events.motion, &cursor->cursorMotion);
    cursor->cursorMotionAbsolute.notify = onCursorMotionAbsolute;
    wl_signal_add(&cursor->wlrCursor->events.motion_absolute, &cursor->cursorMotionAbsolute);
    cursor->cursorButton.notify = onCursorButton;
    wl_signal_add(&cursor->wlrCursor->events.button, &cursor->cursorButton);
    cursor->cursorAxis.notify = onCursorAxis;
    wl_signal_add(&cursor->wlrCursor->events.axis, &cursor->cursorAxis);
    cursor->cursorFrame.notify = onCursorFrame;
    wl_signal_add(&cursor->wlrCursor->events.frame, &cursor->cursorFrame);

    cursor->swipeBegin.notify = onSwipeBegin;
    wl_signal_add(&cursor->wlrCursor->events.swipe_begin, &cursor->swipeBegin);
    cursor->swipeUpdate.notify = onSwipeUpdate;
    wl_signal_add(&cursor->wlrCursor->events.swipe_update, &cursor->swipeUpdate);
    cursor->swipeEnd.notify = onSwipeEnd;
    wl_signal_add(&cursor->wlrCursor->events.swipe_end, &cursor->swipeEnd);
    cursor->pinchBegin.notify = onPinchBegin;
    wl_signal_add(&cursor->wlrCursor->events.pinch_begin, &cursor->pinchBegin);
    cursor->pinchUpdate.notify = onPinchUpdate;
    wl_signal_add(&cursor->wlrCursor->events.pinch_update, &cursor->pinchUpdate);
    cursor->pinchEnd.notify = onPinchEnd;
    wl_signal_add(&cursor->wlrCursor->events.pinch_end, &cursor->pinchEnd);
    cursor->holdBegin.notify = onHoldBegin;
    wl_signal_add(&cursor->wlrCursor->events.hold_begin, &cursor->holdBegin);
    cursor->holdEnd.notify = onHoldEnd;
    wl_signal_add(&cursor->wlrCursor->events.hold_end, &cursor->holdEnd);

    return cursor;
}