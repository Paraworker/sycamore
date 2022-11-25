#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include <wlr/types/wlr_output.h>
#include "sycamore/input/cursor.h"
#include "sycamore/desktop/layer.h"

struct sycamore_output {
    struct wl_list link;
    struct wlr_output *wlr_output;

    struct wl_list layers[LAYERS_ALL];   //sycamore_layer::link
    struct wlr_box usable_area;

    struct wl_listener destroy;
    struct wl_listener frame;
};

/**
 * @brief Center cursor on this output
 */
void output_ensure_cursor(struct sycamore_output *output, struct sycamore_cursor *cursor);

void sycamore_output_destroy(struct sycamore_output *output);

struct sycamore_output *sycamore_output_create(struct wlr_output *wlr_output);

void handle_backend_new_output(struct wl_listener *listener, void *data);

#endif //SYCAMORE_OUTPUT_H