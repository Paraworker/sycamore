#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include <wlr/types/wlr_output.h>
#include "sycamore/desktop/layer.h"

struct sycamore_output {
    struct wl_list link;
    struct wlr_output *wlr_output;

    struct wl_list layers[LAYERS_ALL];   //sycamore_layer::link
    struct wlr_box usable_area;

    struct wl_listener destroy;
    struct wl_listener frame;
};

void handle_backend_new_output(struct wl_listener *listener, void *data);

void output_get_center_coords(struct sycamore_output *output, struct wlr_box *box);

void sycamore_output_destroy(struct sycamore_output *output);

struct sycamore_output *sycamore_output_create(struct wlr_output *wlr_output);

#endif //SYCAMORE_OUTPUT_H