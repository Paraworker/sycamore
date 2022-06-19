#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/layer.h"
#include "sycamore/desktop/shell/layer_shell.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"
#include "sycamore/util/listener.h"

static void handle_layer_map(struct wl_listener *listener, void *data) {
    struct sycamore_layer *layer = wl_container_of(listener, layer, map);

    layer_map(layer);
}

static void handle_layer_unmap(struct wl_listener *listener, void *data) {
    struct sycamore_layer *layer = wl_container_of(listener, layer, unmap);

    layer_unmap(layer);
}

static void handle_layer_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_layer *layer = wl_container_of(listener, layer, destroy);

    layer->layer_surface = NULL;

    layer_destroy(layer);
}

static void handle_layer_surface_commit(struct wl_listener *listener, void *data) {
    struct sycamore_layer *layer = wl_container_of(listener, layer, surface_commit);

    layer_surface_commit(layer);
}

static void handle_new_layer_shell_surface(struct wl_listener *listener, void *data) {
    struct sycamore_layer_shell *layer_shell =
            wl_container_of(listener, layer_shell, new_layer_shell_surface);
    struct wlr_layer_surface_v1 *layer_surface = data;
    struct sycamore_server *server = layer_shell->server;

    struct sycamore_layer *layer = layer_create(server, layer_surface);
    if (!layer) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_layer");
        return;
    }

    enum zwlr_layer_shell_v1_layer layer_type = layer_surface->pending.layer;

    struct wlr_scene_tree *parent = layer_get_scene_tree(server->scene, layer_type);
    layer->scene = wlr_scene_layer_surface_v1_create(
            parent, layer->layer_surface);
    layer->scene->tree->node.data = layer;
    layer->layer_surface->data = layer->scene->tree;

    struct sycamore_output *output = layer->output;
    wl_list_insert(&output->layers[layer_type], &layer->link);
    layer->linked = true;

    listener_connect(&layer->map, &layer_surface->events.map,
                     handle_layer_map);
    listener_connect(&layer->unmap, &layer_surface->events.unmap,
                     handle_layer_unmap);
    listener_connect(&layer->destroy, &layer_surface->events.destroy,
                     handle_layer_destroy);
    listener_connect(&layer->surface_commit, &layer_surface->surface->events.commit,
                     handle_layer_surface_commit);

    // Temporarily set the layer's current state to pending
    // So that we can easily arrange it
    struct wlr_layer_surface_v1_state old_state = layer_surface->current;
    layer_surface->current = layer_surface->pending;
    arrange_layers(output);
    layer_surface->current = old_state;
}

struct sycamore_layer_shell *sycamore_layer_shell_create(
        struct sycamore_server *server, struct wl_display *display) {
    struct sycamore_layer_shell *layer_shell =
            calloc(1, sizeof(struct sycamore_layer_shell));
    if (!layer_shell) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_layer_shell");
        return NULL;
    }

    layer_shell->server = server;

    layer_shell->wlr_layer_shell = wlr_layer_shell_v1_create(display);
    if (!layer_shell->wlr_layer_shell) {
        wlr_log(WLR_ERROR, "Unable to create wlr_layer_shell");
        free(layer_shell);
        return NULL;
    }

    listener_connect(&layer_shell->new_layer_shell_surface, &layer_shell->wlr_layer_shell->events.new_surface,
                     handle_new_layer_shell_surface);

    return layer_shell;
}

void sycamore_layer_shell_destroy(struct sycamore_layer_shell *layer_shell) {
    if (!layer_shell) {
        return;
    }

    listener_disconnect(&layer_shell->new_layer_shell_surface);

    free(layer_shell);
}

