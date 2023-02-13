#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/input/pointer.h"
#include "sycamore/input/seat.h"

static void pointerDestroy(SeatDevice *seatDevice) {
    if (!seatDevice) {
        return;
    }

    wlr_cursor_detach_input_device(seatDevice->seat->cursor->wlrCursor,
                                   seatDevice->wlrDevice);

    free(seatDevice->pointer);
}

Pointer *pointerCreate(Seat *seat, struct wlr_input_device *wlrDevice) {
    Pointer *pointer = calloc(1, sizeof(Pointer));
    if (!pointer) {
        wlr_log(WLR_ERROR, "Unable to allocate Pointer");
        return NULL;
    }

    pointer->base = seatDeviceCreate(seat, wlrDevice, pointer, pointerDestroy);
    if (!pointer->base) {
        wlr_log(WLR_ERROR, "Unable to create seatDevice");
        free(pointer);
        return NULL;
    }

    pointer->wlrPointer = wlr_pointer_from_input_device(wlrDevice);

    return pointer;
}