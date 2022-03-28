#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include "sycamore/server.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/keyboard.h"

struct sycamore_seat {
    struct wlr_seat *wlr_seat;

    struct wl_list keyboards;
    struct sycamore_cursor *cursor;

    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener destroy;

    struct sycamore_server *server;
};

struct sycamore_seat *sycamore_seat_create(struct sycamore_server *server,
        struct wl_display *display, struct wlr_output_layout *output_layout);

void sycamore_seat_destroy(struct sycamore_seat *seat);

void handle_backend_new_input(struct wl_listener *listener, void *data);

#endif //SYCAMORE_SEAT_H
