#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/layer_shell/layer.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"

static void onLayerMap(struct wl_listener *listener, void *data) {
    Layer *layer = wl_container_of(listener, layer, map);
    layerMap(layer);
}

static void onLayerUnmap(struct wl_listener *listener, void *data) {
    Layer *layer = wl_container_of(listener, layer, unmap);
    layerUnmap(layer);
}

static void onLayerDestroy(struct wl_listener *listener, void *data) {
    Layer *layer = wl_container_of(listener, layer, destroy);

    layer->layerSurface = NULL;

    layerDestroy(layer);
}

static void onLayerSurfaceCommit(struct wl_listener *listener, void *data) {
    Layer *layer = wl_container_of(listener, layer, surfaceCommit);

    struct wlr_layer_surface_v1 *layerSurface = layer->layerSurface;
    Output *output = layer->output;
    uint32_t committed = layerSurface->current.committed;

    if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
        enum zwlr_layer_shell_v1_layer layerType = layerSurface->current.layer;
        struct wlr_scene_tree *sceneTree = layerGetSceneTree(server.scene, layerType);
        wlr_scene_node_reparent(&layer->scene->tree->node, sceneTree);
        wl_list_remove(&layer->link);
        wl_list_insert(&output->layers[layerType], &layer->link);
    }

    if (committed || layerSurface->mapped != layer->mapped) {
        layer->mapped = layerSurface->mapped;
        arrangeLayers(output);
    }
}

void layerMap(Layer *layer) {
    if (layer->mapped) {
        return;
    }

    struct wlr_layer_surface_v1 *layerSurface = layer->layerSurface;
    Seat *seat = server.seat;

    // focus on new layerSurface
    if (layerSurface->current.keyboard_interactive &&
        (layerSurface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY ||
         layerSurface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_TOP)) {
        seatSetKeyboardFocus(seat, layerSurface->surface);
        seat->focusedLayer = layer;

        arrangeLayers(layer->output);
    }

    layer->mapped = true;

    seatopPointerRebase(seat);
}

void layerUnmap(Layer *layer) {
    if (!layer->mapped) {
        return;
    }

    Seat *seat = server.seat;

    if (seat->focusedLayer == layer) {
        seat->focusedLayer = NULL;
        View *view = server.focusedView.view;
        if (view) {
            seatSetKeyboardFocus(seat, view->wlrSurface);
        }
    }

    layer->mapped = false;

    seatopPointerRebase(seat);
}

struct wlr_scene_tree *layerGetSceneTree(Scene *scene, enum zwlr_layer_shell_v1_layer type) {
    switch (type) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            return scene->shell.background;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            return scene->shell.bottom;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            return scene->shell.top;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            return scene->shell.overlay;
        default:
            return NULL;
    }
}

void arrange_surface(Output *output, struct wlr_box *fullArea,
        struct wlr_box *usableArea, enum zwlr_layer_shell_v1_layer type) {
    Layer *layer;
    wl_list_for_each(layer, &output->layers[type], link) {
        wlr_scene_layer_surface_v1_configure(layer->scene, fullArea, usableArea);
    }
}

void arrangeLayers(Output *output) {
    struct wlr_box fullArea;
    wlr_output_layout_get_box(server.outputLayout,
                              output->wlrOutput, &fullArea);
    struct wlr_box usableArea = fullArea;

    for (int i = 0; i < LAYERS_ALL; ++i) {
        arrange_surface(output, &fullArea, &usableArea, i);
    }

    output->usableArea = usableArea;
}

Layer *layerCreate(struct wlr_layer_surface_v1 *layerSurface) {
    Layer *layer = calloc(1, sizeof(Layer));
    if (!layer) {
        wlr_log(WLR_ERROR, "Unable to allocate layer");
        return NULL;
    }

    /* Allocate an output for this layer. */
    if (!layerSurface->output) {
        struct wl_list *allOutputs = &server.allOutputs;
        if (wl_list_empty(allOutputs)) {
            wlr_log(WLR_ERROR, "No output for layerSurface");
            free(layer);
            return NULL;
        }

        Output *output = wl_container_of(allOutputs->next, output, link);
        layerSurface->output = output->wlrOutput;
    }

    layer->sceneDesc    = SCENE_DESC_LAYER;
    layer->layerSurface = layerSurface;
    layer->mapped       = false;
    layer->linked       = false;
    layer->output       = layerSurface->output->data;

    layer->map.notify = onLayerMap;
    wl_signal_add(&layerSurface->events.map, &layer->map);
    layer->unmap.notify = onLayerUnmap;
    wl_signal_add(&layerSurface->events.unmap, &layer->unmap);
    layer->destroy.notify = onLayerDestroy;
    wl_signal_add(&layerSurface->events.destroy, &layer->destroy);
    layer->surfaceCommit.notify = onLayerSurfaceCommit;
    wl_signal_add(&layerSurface->surface->events.commit, &layer->surfaceCommit);

    return layer;
}

void layerDestroy(Layer *layer) {
    if (!layer) {
        return;
    }

    if (layer->mapped) {
        layerUnmap(layer);
    }

    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->surfaceCommit.link);

    if (layer->linked) {
        wl_list_remove(&layer->link);
        layer->linked = false;
        arrangeLayers(layer->output);
    }

    if (layer->layerSurface) {
        wlr_layer_surface_v1_destroy(layer->layerSurface);
    }

    free(layer);
}