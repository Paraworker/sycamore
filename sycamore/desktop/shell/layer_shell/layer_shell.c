#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/log.h>
#include "sycamore/defines.h"
#include "sycamore/desktop/shell/layer_shell/layer.h"
#include "sycamore/desktop/shell/layer_shell/layer_shell.h"
#include "sycamore/output/output.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"

static void onNewLayerShellSurface(struct wl_listener *listener, void *data) {
    LayerShell *layerShell = wl_container_of(listener, layerShell, newLayerShellSurface);
    struct wlr_layer_surface_v1 *layerSurface = data;

    Layer *layer = layerCreate(layerSurface);
    if (!layer) {
        wlr_log(WLR_ERROR, "Unable to create Layer");
        return;
    }

    enum zwlr_layer_shell_v1_layer layerType = layerSurface->pending.layer;

    // Add to scene graph
    layer->scene = wlr_scene_layer_surface_v1_create(sceneGetLayerTree(server.scene, layerType), layer->layerSurface);
    layer->scene->tree->node.data = layer;
    layerSurface->surface->data = layer->scene->tree;

    Output *output = layer->output;
    wl_list_insert(&output->layers[layerType], &layer->link);
    layer->linked = true;

    // Temporarily set the layer's current state to pending
    // So that we can easily arrange it
    struct wlr_layer_surface_v1_state oldState = layerSurface->current;
    layerSurface->current = layerSurface->pending;
    arrangeLayers(output);
    layerSurface->current = oldState;
}

struct LayerShell *layerShellCreate(struct wl_display *display) {
    LayerShell *layerShell = calloc(1, sizeof(LayerShell));
    if (!layerShell) {
        wlr_log(WLR_ERROR, "Unable to allocate LayerShell");
        return nullptr;
    }

    layerShell->wlrLayerShell = wlr_layer_shell_v1_create(display, LAYER_SHELL_VERSION);
    if (!layerShell->wlrLayerShell) {
        wlr_log(WLR_ERROR, "Unable to create wlrLayerShell");
        free(layerShell);
        return nullptr;
    }

    layerShell->newLayerShellSurface.notify = onNewLayerShellSurface;
    wl_signal_add(&layerShell->wlrLayerShell->events.new_surface,
                  &layerShell->newLayerShellSurface);

    return layerShell;
}

void layerShellDestroy(LayerShell *layerShell) {
    if (!layerShell) {
        return;
    }

    wl_list_remove(&layerShell->newLayerShellSurface.link);

    free(layerShell);
}