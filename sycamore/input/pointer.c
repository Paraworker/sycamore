#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/input/pointer.h"
#include "sycamore/input/seat.h"

static void sycamore_pointer_destroy(struct sycamore_seat_device *seat_device) {
    if (!seat_device) {
        return;
    }

    wlr_cursor_detach_input_device(seat_device->seat->cursor->wlr_cursor,
                                   seat_device->wlr_device);

    free(seat_device->pointer);
}

struct sycamore_pointer *sycamore_pointer_create(struct sycamore_seat *seat,
        struct wlr_input_device *wlr_device) {
    struct sycamore_pointer *pointer = calloc(1, sizeof(struct sycamore_pointer));
    if (!pointer) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_pointer");
        return NULL;
    }

    pointer->base = seat_device_create(seat, wlr_device, pointer,
                                       sycamore_pointer_destroy);
    if (!pointer->base) {
        wlr_log(WLR_ERROR, "Unable to create seat_device");
        free(pointer);
        return NULL;
    }

    pointer->wlr_pointer = wlr_pointer_from_input_device(wlr_device);

    return pointer;
}