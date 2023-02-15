#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"

static void handlePointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    if (seat->cursor->pressedButtonCount == 0) {
        // If there is no button being pressed
        // we back to basic_full.
        seatopSetBasicFull(seat);
    }
}

static void handlePointerMotion(Seat *seat, uint32_t timeMsec) {
    /* Resizing the grabbed view can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the view
     * on one or two axes, but can also move the view if you resize from the top
     * or left edges (or top-left corner).
     *
     * Note that I took some shortcuts here. In a more fleshed-out compositor,
     * you'd wait for the client to prepare a buffer at the new size, then
     * commit any movement that was prepared. */
    SeatopPointerResizeData *data = &(seat->seatopData.pointerResize);
    struct wlr_cursor *cursor = seat->cursor->wlrCursor;
    View *view = data->viewPtr.view;
    if (!view) {
        return;
    }

    double borderX = cursor->x - data->dx;
    double borderY = cursor->y - data->dy;

    int newLeft   = data->grabGeobox.x;
    int newRight  = data->grabGeobox.x + data->grabGeobox.width;
    int newTop    = data->grabGeobox.y;
    int newBottom = data->grabGeobox.y + data->grabGeobox.height;

    if (data->edges & WLR_EDGE_TOP) {
        newTop = borderY;
        if (newTop >= newBottom) {
            newTop = newBottom - 1;
        }
    } else if (data->edges & WLR_EDGE_BOTTOM) {
        newBottom = borderY;
        if (newBottom <= newTop) {
            newBottom = newTop + 1;
        }
    }
    if (data->edges & WLR_EDGE_LEFT) {
        newLeft = borderX;
        if (newLeft >= newRight) {
            newLeft = newRight - 1;
        }
    } else if (data->edges & WLR_EDGE_RIGHT) {
        newRight = borderX;
        if (newRight <= newLeft) {
            newRight = newLeft + 1;
        }
    }

    struct wlr_box geoBox;
    view->interface->getGeometry(view, &geoBox);

    viewMoveTo(view, newLeft - geoBox.x, newTop - geoBox.y);
    view->interface->setSize(view, newRight - newLeft, newBottom - newTop);
}

static void handleEnd(Seat *seat) {
    SeatopPointerResizeData *data = &(seat->seatopData.pointerResize);
    if (data->viewPtr.view) {
        data->viewPtr.view->interface->setResizing(data->viewPtr.view, false);
        viewPtrDisconnect(&data->viewPtr);
    }
}

static const SeatopImpl impl = {
        .pointerButton = handlePointerButton,
        .pointerMotion = handlePointerMotion,
        .end           = handleEnd,
        .mode          = POINTER_RESIZE,
};

void seatopSetPointerResize(Seat *seat, View *view, uint32_t edges) {
    if (!seatopPointerInteractiveModeCheck(seat, view, POINTER_RESIZE)) {
        return;
    }

    seatopEnd(seat);

    SeatopPointerResizeData *data = &(seat->seatopData.pointerResize);

    viewPtrConnect(&data->viewPtr, view);

    struct wlr_box geoBox;
    view->interface->getGeometry(view, &geoBox);

    double borderX = (view->x + geoBox.x) +
                     ((edges & WLR_EDGE_RIGHT) ? geoBox.width : 0);
    double borderY = (view->y + geoBox.y) +
                     ((edges & WLR_EDGE_BOTTOM) ? geoBox.height : 0);
    data->dx = seat->cursor->wlrCursor->x - borderX;
    data->dy = seat->cursor->wlrCursor->y - borderY;

    data->grabGeobox = geoBox;
    data->grabGeobox.x += view->x;
    data->grabGeobox.y += view->y;

    data->edges = edges;

    seat->seatopImpl = &impl;

    view->interface->setResizing(view, true);

    wlr_seat_pointer_notify_clear_focus(seat->wlrSeat);
    cursorSetImage(seat->cursor, wlr_xcursor_get_resize_name(edges));
}