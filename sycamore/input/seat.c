#include <stdlib.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/keyboard.h"
#include "sycamore/input/libinput.h"
#include "sycamore/input/pointer.h"
#include "sycamore/input/seat.h"
#include "sycamore/input/seat_device.h"
#include "sycamore/server.h"

static void onRequestStartDrag(struct wl_listener *listener, void *data) {
    Seat *seat = wl_container_of(listener, seat, requestStartDrag);
    struct wlr_seat_request_start_drag_event *event = data;

    if (wlr_seat_validate_pointer_grab_serial(seat->wlrSeat, event->origin, event->serial)) {
        wlr_seat_start_pointer_drag(seat->wlrSeat, event->drag, event->serial);
        return;
    }

    struct wlr_touch_point *point;
    if (wlr_seat_validate_touch_grab_serial(seat->wlrSeat,
                                            event->origin, event->serial, &point)) {
        wlr_seat_start_touch_drag(seat->wlrSeat,
                                  event->drag, event->serial, point);
        return;
    }

    /* TODO: tablet grabs */

    wlr_data_source_destroy(event->drag->source);
}

static void onDragDestroy(struct wl_listener *listener, void *data) {
    Drag *drag = wl_container_of(listener, drag, destroy);

    wl_list_remove(&drag->destroy.link);
    drag->wlrDrag->data = nullptr;

    free(drag);
}

static void onDragIconDestroy(struct wl_listener *listener, void *data) {
    DragIcon *icon = wl_container_of(listener, icon, destroy);

    wl_list_remove(&icon->destroy.link);
    icon->wlrDragIcon->data = nullptr;

    free(icon);
}

static void onStartDrag(struct wl_listener *listener, void *data) {
    Seat *seat = wl_container_of(listener, seat, startDrag);
    struct wlr_drag *wlrDrag = data;
    Drag *drag = calloc(1, sizeof(Drag));
    if (!drag) {
        wlr_log(WLR_ERROR, "Unable to allocate Drag");
        return;
    }

    drag->seat    = seat;
    drag->wlrDrag = wlrDrag;
    wlrDrag->data = drag;

    drag->destroy.notify = onDragDestroy;
    wl_signal_add(&wlrDrag->events.destroy, &drag->destroy);

    struct wlr_drag_icon *wlrDragIcon = wlrDrag->icon;
    if (wlrDragIcon) {
        // Create DragIcon
        DragIcon *icon = calloc(1, sizeof(DragIcon));
        if (!icon) {
            wlr_log(WLR_ERROR, "Failed to allocate DragIcon");
            return;
        }

        icon->tree = wlr_scene_drag_icon_create(server.scene->dragIcons, wlrDragIcon);
        if (!icon->tree) {
            wlr_log(WLR_ERROR, "Failed to create scene drag icon");
            free(icon);
            return;
        }

        icon->tree->node.data = icon;
        icon->sceneDesc       = SCENE_DESC_DRAG_ICON;
        icon->wlrDragIcon     = wlrDragIcon;
        wlrDragIcon->data     = icon;

        icon->destroy.notify = onDragIconDestroy;
        wl_signal_add(&wlrDragIcon->events.destroy, &icon->destroy);

        seatDragIconUpdatePosition(seat, icon);
    }

    seatopSetDefault(seat);
}

void seatDragIconUpdatePosition(Seat *seat, DragIcon *icon) {
    struct wlr_drag_icon *wlrIcon = icon->wlrDragIcon;
    struct wlr_cursor *cursor = seat->cursor->wlrCursor;

    switch (wlrIcon->drag->grab_type) {
        case WLR_DRAG_GRAB_KEYBOARD:
            return;
        case WLR_DRAG_GRAB_KEYBOARD_POINTER:
            wlr_scene_node_set_position(&icon->tree->node, (int)cursor->x, (int)cursor->y);
            break;
        case WLR_DRAG_GRAB_KEYBOARD_TOUCH:
            /* TODO */
            break;
    }
}

void seatUpdateCapabilities(Seat *seat) {
    uint32_t caps = 0;
    SeatDevice *seatDevice;
    wl_list_for_each(seatDevice, &seat->devices, link) {
        switch (seatDevice->wlrDevice->type) {
            case WLR_INPUT_DEVICE_KEYBOARD:
                caps |= WL_SEAT_CAPABILITY_KEYBOARD;
                break;
            case WLR_INPUT_DEVICE_POINTER:
                caps |= WL_SEAT_CAPABILITY_POINTER;
                break;
            case WLR_INPUT_DEVICE_TOUCH:
                caps |= WL_SEAT_CAPABILITY_TOUCH;
                break;
            case WLR_INPUT_DEVICE_TABLET_TOOL:
                caps |= WL_SEAT_CAPABILITY_POINTER;
                break;
            case WLR_INPUT_DEVICE_SWITCH:
            case WLR_INPUT_DEVICE_TABLET_PAD:
                break;
        }
    }

    wlr_seat_set_capabilities(seat->wlrSeat, caps);

    // Disable cursor if seat doesn't have pointer capability.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
        cursorDisable(seat->cursor);
    }
}

static void seatConfigureKeyboard(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new keyboard: %s", device->name);

    Keyboard *keyboard = keyboardCreate(seat, device);
    if (!keyboard) {
        wlr_log(WLR_ERROR, "Unable to create Keyboard");
        return;
    }

    keyboardConfigure(keyboard);
    wlr_seat_set_keyboard(seat->wlrSeat, keyboard->wlrKeyboard);
    wl_list_insert(&seat->devices, &keyboard->base->link);
}

static void seatConfigurePointer(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new pointer: %s", device->name);

    Pointer *pointer = pointerCreate(seat, device);
    if (!pointer) {
        wlr_log(WLR_ERROR, "Unable to create Pointer");
        return;
    }

    wl_list_insert(&seat->devices, &pointer->base->link);

    touchpadSetTapToClick(device);
    touchpadSetNaturalScroll(device);
    touchpadSetAccelSpeed(device, 0.3);
}

static void seatConfigureTouch(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new touch: %s", device->name);
    
    /* TODO */
}

static void seatConfigureTabletTool(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet tool: %s", device->name);

    wlr_cursor_attach_input_device(seat->cursor->wlrCursor, device);
    /* TODO */
}

static void seatConfigureTabletPad(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new tablet pad: %s", device->name);

    /* TODO */
}

static void seatConfigureSwitch(Seat *seat, struct wlr_input_device *device) {
    wlr_log(WLR_DEBUG, "new switch: %s", device->name);

    /* TODO */
}

static void (*seatConfigureDevice[])(Seat *seat, struct wlr_input_device *device) = {
        seatConfigureKeyboard,
        seatConfigurePointer,
        seatConfigureTouch,
        seatConfigureTabletTool,
        seatConfigureTabletPad,
        seatConfigureSwitch,
};

void onBackendNewInput(struct wl_listener *listener, void *data) {
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct wlr_input_device *device = data;
    Seat *seat = server.seat;

    seatConfigureDevice[device->type](seat, device);

    seatUpdateCapabilities(seat);
}

static void onSeatRequestSetCursor(struct wl_listener *listener, void *data) {
    Seat *seat = wl_container_of(listener, seat, requestSetCursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    struct wlr_seat_client *focusedClient = seat->wlrSeat->pointer_state.focused_client;
    if (focusedClient != event->seat_client) {
        return;
    }

    cursorSetSurface(seat->cursor, event->surface, event->hotspot_x, event->hotspot_y);
}

static void onSeatRequestSetSelection(struct wl_listener *listener, void *data) {
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but we always honor
     */
    Seat *seat = wl_container_of(listener, seat, requestSetSelection);
    struct wlr_seat_request_set_selection_event *event = data;

    wlr_seat_set_selection(seat->wlrSeat, event->source, event->serial);
}

static void onSeatRequestSetPrimarySelection(struct wl_listener *listener, void *data) {
    Seat *seat = wl_container_of(listener, seat, requestSetPrimarySelection);
    struct wlr_seat_request_set_primary_selection_event *event = data;

    wlr_seat_set_primary_selection(seat->wlrSeat, event->source, event->serial);
}

static void onSeatDestroy(struct wl_listener *listener, void *data) {
    Seat *seat = wl_container_of(listener, seat, destroy);

    seat->wlrSeat = nullptr;
    server.seat   = nullptr;

    seatDestroy(seat);
}

void seatPointerUpdateFocus(Seat *seat, uint32_t timeMsec) {
    Cursor *cursor = seat->cursor;

    double sx, sy;
    struct wlr_surface *surface =
            getSurfaceFromNode(FIND_NODE(cursor->wlrCursor->x, cursor->wlrCursor->y, &sx, &sy));

    if (!surface) {
        wlr_seat_pointer_clear_focus(seat->wlrSeat);
        cursorSetImage(cursor, "default");
        return;
    }

    wlr_seat_pointer_notify_enter(seat->wlrSeat, surface, sx, sy);
    wlr_seat_pointer_notify_motion(seat->wlrSeat, timeMsec, sx, sy);
}

void seatSetKeyboardFocus(Seat *seat, struct wlr_surface *surface) {
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->wlrSeat);
    if (!keyboard) {
        wlr_seat_keyboard_notify_enter(seat->wlrSeat, surface, nullptr, 0, nullptr);
        return;
    }

    wlr_seat_keyboard_notify_enter(seat->wlrSeat, surface,
                                   keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

void seatDestroy(Seat *seat) {
    if (!seat) {
        return;
    }

    SeatDevice *seatDevice, *next;
    wl_list_for_each_safe(seatDevice, next, &seat->devices, link) {
        seatDeviceDestroy(seatDevice);
    }

    seatopEnd(seat);

    wl_list_remove(&seat->destroy.link);
    wl_list_remove(&seat->requestSetCursor.link);
    wl_list_remove(&seat->requestSetSelection.link);
    wl_list_remove(&seat->requestSetPrimarySelection.link);
    wl_list_remove(&seat->requestStartDrag.link);
    wl_list_remove(&seat->startDrag.link);

    if (seat->cursor) {
        cursorDestroy(seat->cursor);
    }

    if (seat->wlrSeat) {
        wlr_seat_destroy(seat->wlrSeat);
    }

    free(seat);
}

Seat *seatCreate(struct wl_display *display, struct wlr_output_layout *layout) {
    Seat *seat = calloc(1, sizeof(Seat));
    if (!seat) {
        wlr_log(WLR_ERROR, "Unable to allocate Seat");
        return nullptr;
    }

    seat->seatopImpl = nullptr;
    wl_list_init(&seat->devices);

    seat->wlrSeat = wlr_seat_create(display, "seat0");
    if (!seat->wlrSeat) {
        wlr_log(WLR_ERROR, "Unable to create wlrSeat");
        free(seat);
        return nullptr;
    }

    seat->cursor = cursorCreate(seat, display, layout);
    if (!seat->cursor) {
        wlr_log(WLR_ERROR, "Unable to create Cursor");
        wlr_seat_destroy(seat->wlrSeat);
        free(seat);
        return nullptr;
    }

    seat->requestSetCursor.notify = onSeatRequestSetCursor;
    wl_signal_add(&seat->wlrSeat->events.request_set_cursor,
                  &seat->requestSetCursor);
    seat->requestSetSelection.notify = onSeatRequestSetSelection;
    wl_signal_add(&seat->wlrSeat->events.request_set_selection,
                  &seat->requestSetSelection);
    seat->requestSetPrimarySelection.notify = onSeatRequestSetPrimarySelection;
    wl_signal_add(&seat->wlrSeat->events.request_set_primary_selection,
                  &seat->requestSetPrimarySelection);
    seat->requestStartDrag.notify = onRequestStartDrag;
    wl_signal_add(&seat->wlrSeat->events.request_start_drag,
                  &seat->requestStartDrag);
    seat->startDrag.notify = onStartDrag;
    wl_signal_add(&seat->wlrSeat->events.start_drag,
                  &seat->startDrag);
    seat->destroy.notify = onSeatDestroy;
    wl_signal_add(&seat->wlrSeat->events.destroy,
                  &seat->destroy);

    seatopSetDefault(seat);

    return seat;
}

bool seatopPointerInteractiveModeCheck(Seat *seat, View *view, SeatopMode mode) {
    /* This fuction is used for checking whether an
     * 'pointer interactive' seatop mode should begin. including:
     *
     * POINTER_MOVE
     * POINTER_RESIZE
     */

    // Don't renter.
    if (seat->seatopImpl->mode == mode) {
        return false;
    }

    // Deny pointerMove/pointerResize from maximized/fullscreen view.
    if (view->isMaximized || view->isFullscreen) {
        return false;
    }

    // Deny pointerMove/pointerResize from unfocused view or there is no focused view.
    if (view != server.focusedView.view) {
        return false;
    }

    return true;
}