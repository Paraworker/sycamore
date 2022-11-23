#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/layer.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"

void layer_map(struct sycamore_layer *layer) {
    if (layer->mapped) {
        return;
    }

    struct wlr_layer_surface_v1 *layer_surface = layer->layer_surface;
    struct sycamore_seat *seat = server.seat;

    // focus on new layer_surface
    if (layer_surface->current.keyboard_interactive &&
        (layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY ||
         layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_TOP)) {
        seat_set_keyboard_focus(seat, layer_surface->surface);
        seat->focused_layer = layer;

        arrange_layers(layer->output);
    }

    layer->mapped = true;

    pointer_rebase(seat);
}

void layer_unmap(struct sycamore_layer *layer) {
    if (!layer->mapped) {
        return;
    }

    struct sycamore_seat *seat = server.seat;

    if (seat->focused_layer == layer) {
        seat->focused_layer = NULL;
        struct sycamore_view *view = server.focused_view.view;
        if (view) {
            seat_set_keyboard_focus(seat, view->wlr_surface);
        }
    }

    layer->mapped = false;

    pointer_rebase(seat);
}

void layer_surface_commit(struct sycamore_layer *layer) {
    struct wlr_layer_surface_v1 *layer_surface = layer->layer_surface;
    struct sycamore_output *output = layer->output;
    uint32_t committed = layer_surface->current.committed;

    if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
        enum zwlr_layer_shell_v1_layer layer_type = layer_surface->current.layer;
        struct wlr_scene_tree *scene_tree = layer_get_scene_tree(server.scene, layer_type);
        wlr_scene_node_reparent(&layer->scene->tree->node, scene_tree);
        wl_list_remove(&layer->link);
        wl_list_insert(&output->layers[layer_type], &layer->link);
    }

    if (committed || layer_surface->mapped != layer->mapped) {
        layer->mapped = layer_surface->mapped;
        arrange_layers(output);
    }
}

struct wlr_scene_tree *layer_get_scene_tree(struct sycamore_scene *scene,
        enum zwlr_layer_shell_v1_layer type) {
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

void arrange_surface(struct sycamore_output *output, struct wlr_box *full_area,
                     struct wlr_box *usable_area, enum zwlr_layer_shell_v1_layer type) {
    struct sycamore_layer *layer;
    wl_list_for_each(layer, &output->layers[type], link) {
        wlr_scene_layer_surface_v1_configure(layer->scene, full_area, usable_area);
    }
}

void arrange_layers(struct sycamore_output *output) {
    struct wlr_box full_area;
    wlr_output_layout_get_box(server.output_layout,
                              output->wlr_output, &full_area);
    struct wlr_box usable_area = full_area;

    for (int i = 0; i < LAYERS_ALL; ++i) {
        arrange_surface(output, &full_area, &usable_area, i);
    }

    output->usable_area = usable_area;
}

struct sycamore_layer *layer_create(struct wlr_layer_surface_v1 *layer_surface) {
    struct sycamore_layer *layer = calloc(1, sizeof(struct sycamore_layer));
    if (!layer) {
        wlr_log(WLR_ERROR, "Unable to allocate layer");
        return NULL;
    }

    /* Allocate an output for this layer. */
    if (!layer_surface->output) {
        struct wl_list *all_outputs = &server.all_outputs;
        if (wl_list_empty(all_outputs)) {
            wlr_log(WLR_ERROR, "No output for layer_surface");
            free(layer);
            return NULL;
        }

        struct sycamore_output *output = wl_container_of(all_outputs->next, output, link);
        layer_surface->output = output->wlr_output;
    }

    layer->scene_descriptor = SCENE_DESC_LAYER;
    layer->layer_surface = layer_surface;
    layer->mapped = false;
    layer->linked = false;
    layer->output = layer_surface->output->data;

    return layer;
}

void layer_destroy(struct sycamore_layer *layer) {
    if (!layer) {
        return;
    }

    if (layer->mapped) {
        layer_unmap(layer);
    }

    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->surface_commit.link);

    if (layer->linked) {
        wl_list_remove(&layer->link);
        layer->linked = false;
        arrange_layers(layer->output);
    }

    if (layer->layer_surface) {
        wlr_layer_surface_v1_destroy(layer->layer_surface);
    }

    free(layer);
}