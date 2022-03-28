#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/layer.h"
#include "sycamore/output/output.h"

void sycamore_layer_map(struct sycamore_layer *layer) {
    layer->mapped = true;
}

void sycamore_layer_unmap(struct sycamore_layer *layer) {
    layer->mapped = false;
}

struct wlr_scene_layer_surface_v1 *sycamore_layer_add_to_scene(struct sycamore_scene *scene,
                                                            struct sycamore_layer *layer) {
    struct wlr_scene_node *parent = NULL;
    switch (layer->layer_type) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            parent = &scene->trees.shell_background->node;
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            parent = &scene->trees.shell_button->node;
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            parent = &scene->trees.shell_top->node;
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            parent = &scene->trees.shell_overlay->node;
            break;
    }

    struct wlr_scene_layer_surface_v1 *scene_layer_surface = wlr_scene_layer_surface_v1_create(
            parent, layer->wlr_layer_surface);
    scene_layer_surface->node->data = layer;
    layer->wlr_layer_surface->surface->data = scene_layer_surface->node;

    return scene_layer_surface;
}

struct sycamore_layer* sycamore_layer_create(struct sycamore_server *server, struct wlr_layer_surface_v1 *layer_surface) {
    struct sycamore_layer *sycamore_layer = calloc(1, sizeof(struct sycamore_layer));
    if (!sycamore_layer) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_layer");
        return NULL;
    }

    struct sycamore_output *output = NULL;
    if (!layer_surface->output) {
        output = wl_container_of(server->all_outputs.next, output, link);
        layer_surface->output = output->wlr_output;
    } else {
        output = layer_surface->output->data;
    }

    sycamore_layer->scene_descriptor = SCENE_DESC_LAYER;
    sycamore_layer->layer_type = layer_surface->pending.layer;
    sycamore_layer->wlr_layer_surface = layer_surface;
    sycamore_layer->server = server;
    sycamore_layer->mapped = false;

    sycamore_layer->scene_layer_surface = sycamore_layer_add_to_scene(server->scene, sycamore_layer);

    sycamore_layer->map.notify = handle_sycamore_layer_map;
    wl_signal_add(&layer_surface->events.map, &sycamore_layer->map);
    sycamore_layer->unmap.notify = handle_sycamore_layer_unmap;
    wl_signal_add(&layer_surface->events.unmap, &sycamore_layer->unmap);
    sycamore_layer->destroy.notify = handle_sycamore_layer_destroy;
    wl_signal_add(&layer_surface->events.destroy, &sycamore_layer->destroy);

    wl_list_insert(&output->layers[sycamore_layer->layer_type],
                   &sycamore_layer->link);

    struct wlr_box full_area;
    wlr_output_layout_get_box(server->output_layout, output->wlr_output, &full_area);
    wlr_scene_layer_surface_v1_configure(sycamore_layer->scene_layer_surface,
                                         &full_area, &output->usable_area);

    return sycamore_layer;
}

void sycamore_layer_destroy(struct sycamore_layer *layer) {
    if (!layer) {
        return;
    }

    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->link);

    if (layer->wlr_layer_surface) {
        wlr_layer_surface_v1_destroy(layer->wlr_layer_surface);
    }

    free(layer);
}