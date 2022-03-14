#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include "sycamore/server.h"
#include <wlr/types/wlr_output.h>

struct sycamore_output {
    struct wl_list link;
    struct wlr_output *wlr_output;
    struct wl_listener frame;

    struct sycamore_server* server;
};

/* This event is raised by the backend when a new output (aka a display or monitor) becomes available. */
void handle_backend_new_output(struct wl_listener *listener, void *data);

#endif //SYCAMORE_OUTPUT_H
