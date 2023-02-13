#ifndef SYCAMORE_POINTER_H
#define SYCAMORE_POINTER_H

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>

typedef struct Pointer    Pointer;
typedef struct Seat       Seat;
typedef struct SeatDevice SeatDevice;

struct Pointer {
    SeatDevice         *base;
    struct wlr_pointer *wlrPointer;
};

Pointer *pointerCreate(Seat *seat, struct wlr_input_device *wlrDevice);

#endif //SYCAMORE_POINTER_H