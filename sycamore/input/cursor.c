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

static inline void hideCursor(Cursor *cursor) {
    cursor->image = NULL;
    wlr_cursor_set_image(cursor->wlrCursor, NULL, 0, 0, 0, 0, 0, 0);
}

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

void cursorDisable(Cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    cursor->enabled = false;

    hideCursor(cursor);
    wlr_seat_pointer_notify_clear_focus(cursor->seat->wlrSeat);
}

void cursorEnable(Cursor *cursor) {
    if (cursor->enabled) {
        return;
    }

    cursor->enabled = true;

    seatopPointerRebase(cursor->seat);
}

void cursorSetImage(Cursor *cursor, const char *image) {
    if (!cursor->enabled) {
        return;
    }

    if (!image) {
        hideCursor(cursor);
        return;
    }

    // Avoid setting duplicate image
    if (cursor->image && strcmp(cursor->image, image) == 0) {
        return;
    }

    cursor->image = image;

    wlr_xcursor_manager_set_cursor_image(cursor->xcursorManager, image, cursor->wlrCursor);
}

void cursorSetSurface(Cursor *cursor, struct wlr_surface *surface, int32_t hotspotX, int32_t hotspotY) {
    if (!cursor->enabled) {
        return;
    }

    cursor->image = NULL;

    wlr_cursor_set_surface(cursor->wlrCursor, surface, hotspotX, hotspotY);
}

void cursorRefresh(Cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    cursorWarp(cursor, cursor->wlrCursor->x, cursor->wlrCursor->y);

    if (!cursor->image) {
        cursor->image = "default";
    }

    wlr_xcursor_manager_set_cursor_image(cursor->xcursorManager, cursor->image, cursor->wlrCursor);
}

void cursorRebase(Cursor *cursor) {
    if (!cursor->enabled) {
        return;
    }

    seatopPointerRebase(cursor->seat);
}

void cursorWarp(Cursor *cursor, double lx, double ly) {
    wlr_cursor_warp(cursor->wlrCursor, NULL, lx, ly);
}

Output *cursorAtOutput(Cursor *cursor) {
    struct wlr_cursor *wlrCursor = cursor->wlrCursor;
    struct wlr_output *output = wlr_output_layout_output_at(server.outputLayout, wlrCursor->x, wlrCursor->y);
    if (!output) {
        return NULL;
    }

    return output->data;
}

static void onFrame(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, frame);
    wlr_seat_pointer_notify_frame(cursor->seat->wlrSeat);
}

static void onMotion(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, motion);
    cursorEnable(cursor);

    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(cursor->wlrCursor, &event->pointer->base, event->delta_x, event->delta_y);
    seatopPointerMotion(cursor->seat, event->time_msec);
}

static void onMotionAbsolute(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, motionAbsolute);
    cursorEnable(cursor);

    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(cursor->wlrCursor, &event->pointer->base, event->x, event->y);
    seatopPointerMotion(cursor->seat, event->time_msec);
}

static void onButton(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, button);
    cursorEnable(cursor);

    struct wlr_pointer_button_event *event = data;

    if (event->state == WLR_BUTTON_PRESSED) {
        ++(cursor->pressedButtonCount);
    } else if (cursor->pressedButtonCount > 0) {
        --(cursor->pressedButtonCount);
    }

    seatopPointerButton(cursor->seat, event);
}

static void onAxis(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, axis);
    cursorEnable(cursor);

    seatopPointerAxis(cursor->seat, data);
}

static void onSwipeBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeBegin);
    cursorEnable(cursor);

    seatopPointerSwipeBegin(cursor->seat, data);
}

static void onSwipeUpdate(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeUpdate);
    cursorEnable(cursor);

    seatopPointerSwipeUpdate(cursor->seat, data);
}

static void onSwipeEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, swipeEnd);
    cursorEnable(cursor);

    seatopPointerSwipeEnd(cursor->seat, data);
}

static void onPinchBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchBegin);
    cursorEnable(cursor);

    seatopPointerPinchBegin(cursor->seat, data);
}

static void onPinchUpdate(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchUpdate);
    cursorEnable(cursor);

    seatopPointerPinchUpdate(cursor->seat, data);
}

static void onPinchEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, pinchEnd);
    cursorEnable(cursor);

    seatopPointerPinchEnd(cursor->seat, data);
}

static void onHoldBegin(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, holdBegin);
    cursorEnable(cursor);

    seatopPointerHoldBegin(cursor->seat, data);
}

static void onHoldEnd(struct wl_listener *listener, void *data) {
    Cursor *cursor = wl_container_of(listener, cursor, holdEnd);
    cursorEnable(cursor);

    seatopPointerHoldEnd(cursor->seat, data);
}

void cursorDestroy(Cursor *cursor) {
    if (!cursor) {
        return;
    }

    wl_list_remove(&cursor->frame.link);

    wl_list_remove(&cursor->motion.link);
    wl_list_remove(&cursor->motionAbsolute.link);
    wl_list_remove(&cursor->button.link);
    wl_list_remove(&cursor->axis.link);

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

    cursor->enabled            = false;
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

    cursor->frame.notify = onFrame;
    wl_signal_add(&cursor->wlrCursor->events.frame, &cursor->frame);

    cursor->motion.notify = onMotion;
    wl_signal_add(&cursor->wlrCursor->events.motion, &cursor->motion);
    cursor->motionAbsolute.notify = onMotionAbsolute;
    wl_signal_add(&cursor->wlrCursor->events.motion_absolute, &cursor->motionAbsolute);
    cursor->button.notify = onButton;
    wl_signal_add(&cursor->wlrCursor->events.button, &cursor->button);
    cursor->axis.notify = onAxis;
    wl_signal_add(&cursor->wlrCursor->events.axis, &cursor->axis);

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