#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/layer.h"
#include "sycamore/output/output.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"

void layer_map(struct sycamore_layer *layer) {
    if (layer->mapped) {
        return;
    }

    struct wlr_layer_surface_v1 *layer_surface = layer->layer_surface;
    struct sycamore_seat *seat = layer->server->seat;

    // focus on new layer_surface
    if (layer_surface->current.keyboard_interactive &&
        (layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY ||
         layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_TOP)) {
        seat_set_keyboard_focus(seat, layer_surface->surface);
        seat->focused_layer = layer;

        arrange_layers(layer->output);
    }

    layer->mapped = true;

    seat->seatop_impl->cursor_rebase(seat);
}

void layer_unmap(struct sycamore_layer *layer) {
    if (!layer->mapped) {
        return;
    }

    struct sycamore_server *server = layer->server;
    struct sycamore_seat *seat = server->seat;

    if (seat->focused_layer == layer) {
        seat->focused_layer = NULL;
        struct sycamore_view *view = server->focused_view.view;
        if (view) {
            seat_set_keyboard_focus(seat, view->wlr_surface);
        }
    }

    layer->mapped = false;

    seat->seatop_impl->cursor_rebase(seat);
}

struct wlr_scene_tree *layer_get_scene_tree(struct sycamore_scene *root,
        struct sycamore_layer *layer) {
    switch (layer->layer_type) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            return root->trees.shell_background;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            return root->trees.shell_button;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            return root->trees.shell_top;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            return root->trees.shell_overlay;
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
    wlr_output_layout_get_box(output->server->output_layout,
                              output->wlr_output, &full_area);
    struct wlr_box usable_area = full_area;

    for (int i = 0; i < LAYERS_ALL; ++i) {
        arrange_surface(output, &full_area, &usable_area, i);
    }

    output->usable_area = usable_area;
}

struct sycamore_layer *sycamore_layer_create(struct sycamore_server *server, struct wlr_layer_surface_v1 *layer_surface) {
    struct sycamore_layer *sycamore_layer = calloc(1, sizeof(struct sycamore_layer));
    if (!sycamore_layer) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_layer");
        return NULL;
    }

    /* Allocate an output for this layer. */
    if (!layer_surface->output) {
        struct wl_list *all_outputs = &server->all_outputs;
        if (wl_list_empty(all_outputs)) {
            wlr_log(WLR_ERROR, "No output for layer_surface");
            free(sycamore_layer);
            return NULL;
        }

        struct sycamore_output *output = wl_container_of(all_outputs->next, output, link);
        layer_surface->output = output->wlr_output;
    }

    sycamore_layer->scene_descriptor = SCENE_DESC_LAYER;
    sycamore_layer->layer_surface = layer_surface;
    sycamore_layer->layer_type = layer_surface->pending.layer;
    sycamore_layer->mapped = false;
    sycamore_layer->linked = false;
    sycamore_layer->output = layer_surface->output->data;
    sycamore_layer->server = server;

    sycamore_layer->map.notify = handle_sycamore_layer_map;
    wl_signal_add(&layer_surface->events.map, &sycamore_layer->map);
    sycamore_layer->unmap.notify = handle_sycamore_layer_unmap;
    wl_signal_add(&layer_surface->events.unmap, &sycamore_layer->unmap);
    sycamore_layer->destroy.notify = handle_sycamore_layer_destroy;
    wl_signal_add(&layer_surface->events.destroy, &sycamore_layer->destroy);

    return sycamore_layer;
}

void sycamore_layer_destroy(struct sycamore_layer *layer) {
    if (!layer) {
        return;
    }

    if (layer->mapped) {
        layer_unmap(layer);
    }

    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);

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