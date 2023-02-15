#ifndef SYCAMORE_SEAT_DEVICE_H
#define SYCAMORE_SEAT_DEVICE_H

#include <wlr/types/wlr_input_device.h>

typedef struct Seat       Seat;
typedef struct SeatDevice SeatDevice;
typedef struct Keyboard   Keyboard;
typedef struct Pointer    Pointer;

typedef void (*DerivedSeatDeviceDestroy)(SeatDevice *seatDevice);

struct SeatDevice {
    struct wlr_input_device *wlrDevice;
    struct wl_list link; // Seat::devices

    union {
        void     *derivedDevice;
        Pointer  *pointer;
        Keyboard *keyboard;
    };

    DerivedSeatDeviceDestroy derivedDestroy;

    struct wl_listener destroy;

    Seat *seat;
};

SeatDevice *seatDeviceCreate(Seat *seat, struct wlr_input_device *wlrDevice,
        void *derivedDevice, DerivedSeatDeviceDestroy derivedDestroy);

void seatDeviceDestroy(SeatDevice *seatDevice);

#endif //SYCAMORE_SEAT_DEVICE_H