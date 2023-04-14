#ifndef SYCAMORE_LAYER_SHELL_H
#define SYCAMORE_LAYER_SHELL_H

#include <wlr/types/wlr_layer_shell_v1.h>

typedef struct LayerShell LayerShell;

struct LayerShell {
    struct wlr_layer_shell_v1 *wlrLayerShell;
    struct wl_listener newLayerShellSurface;
};

LayerShell *layerShellCreate(struct wl_display *display);

void layerShellDestroy(LayerShell *layerShell);

#endif //SYCAMORE_LAYER_SHELL_H