#include <stdlib.h>
#include "sycamore/input/seat.h"
#include "sycamore/input/seat_device.h"

static void onSeatDeviceDestroy(struct wl_listener *listener, void *data) {
    SeatDevice *seatDevice = wl_container_of(listener, seatDevice, destroy);
    Seat *seat = seatDevice->seat;

    seatDeviceDestroy(seatDevice);
    seatUpdateCapabilities(seat);
}

SeatDevice *seatDeviceCreate(Seat *seat, struct wlr_input_device *wlrDevice,
        void *derivedDevice, DerivedSeatDeviceDestroy derivedDestroy) {
    SeatDevice *seatDevice = calloc(1, sizeof(SeatDevice));
    if (!seatDevice) {
        return NULL;
    }

    seatDevice->wlrDevice      = wlrDevice;
    seatDevice->derivedDevice  = derivedDevice;
    seatDevice->derivedDestroy = derivedDestroy;
    seatDevice->seat           = seat;

    seatDevice->destroy.notify = onSeatDeviceDestroy;
    wl_signal_add(&wlrDevice->events.destroy, &seatDevice->destroy);

    return seatDevice;
}

void seatDeviceDestroy(SeatDevice *seatDevice) {
    if (!seatDevice) {
        return;
    }

    wl_list_remove(&seatDevice->destroy.link);
    wl_list_remove(&seatDevice->link);

    if (seatDevice->derivedDestroy) {
        seatDevice->derivedDestroy(seatDevice);
    }

    free(seatDevice);
}