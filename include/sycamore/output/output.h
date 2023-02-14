#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include <wlr/types/wlr_output.h>
#include "sycamore/input/cursor.h"
#include "sycamore/desktop/shell/layer_shell/layer.h"

typedef struct Output Output;

struct Output {
    struct wl_list    link;
    struct wlr_output *wlrOutput;

    struct wl_list layers[LAYERS_ALL];   //Layer::link
    struct wlr_box usableArea;

    struct wl_listener destroy;
    struct wl_listener frame;
    struct wl_listener requestState;
};

/**
 * @brief Center cursor on this output
 */
void outputEnsureCursor(Output *output, Cursor *cursor);

Output *outputCreate(struct wlr_output *wlrOutput);

void outputDestroy(Output *output);

void onBackendNewOutput(struct wl_listener *listener, void *data);

#endif //SYCAMORE_OUTPUT_H