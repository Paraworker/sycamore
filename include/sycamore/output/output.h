#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include "sycamore/server.h"
#include <wlr/types/wlr_output.h>

struct sycamore_output {
    struct wl_list link;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;

    struct sycamore_server* server;
};

/* This event is raised by the backend when a new output (aka a display or monitor) becomes available. */
void handle_backend_new_output(struct wl_listener *listener, void *data);

struct sycamore_output* sycamore_output_create(struct sycamore_server* server,
                                               struct wlr_output *wlr_output);
void sycamore_output_destroy(struct sycamore_output* output);

#endif //SYCAMORE_OUTPUT_H
