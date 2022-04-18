#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/input/pointer.h"
#include "sycamore/input/seat.h"

static void handle_pointer_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_seat_device *device = wl_container_of(listener, device, destroy);
    struct sycamore_seat *seat = device->seat;
    sycamore_pointer_destroy(device->pointer);
    seat_update_capabilities(seat);
}

void sycamore_pointer_destroy(struct sycamore_pointer *pointer) {
    if (!pointer) {
        return;
    }

    if (pointer->base) {
        seat_device_destroy(pointer->base);
    }

    free(pointer);
}

struct sycamore_pointer *sycamore_pointer_create(struct sycamore_seat *seat,
        struct sycamore_cursor *cursor, struct wlr_input_device *wlr_device) {
    struct sycamore_pointer *pointer = calloc(1, sizeof(struct sycamore_pointer));
    if (!pointer) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_pointer");
        return NULL;
    }

    pointer->base = seat_device_create(seat, wlr_device, pointer,
                                       handle_pointer_destroy);
    if (!pointer->base) {
        wlr_log(WLR_ERROR, "Unable to create seat_device");
        free(pointer);
        return NULL;
    }

    pointer->wlr_pointer = wlr_device->pointer;
    pointer->cursor = cursor;

    return pointer;
}
