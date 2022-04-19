#ifndef SYCAMORE_POINTER_H
#define SYCAMORE_POINTER_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>

struct sycamore_seat;

struct sycamore_pointer {
    struct sycamore_seat_device *base;
    struct wlr_pointer *wlr_pointer;
};

struct sycamore_pointer *sycamore_pointer_create(struct sycamore_seat *seat,
        struct wlr_input_device *wlr_device);

#endif //SYCAMORE_POINTER_H
