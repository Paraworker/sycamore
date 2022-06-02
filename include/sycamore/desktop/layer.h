#ifndef SYCAMORE_LAYER_H
#define SYCAMORE_LAYER_H

#include <wlr/types/wlr_layer_shell_v1.h>
#include "sycamore/output/scene.h"

#define LAYERS_ALL 4

struct sycamore_output;
struct sycamore_server;

struct sycamore_layer {
    enum scene_descriptor_type scene_descriptor;    //must be first
    struct wlr_layer_surface_v1 *layer_surface;
    enum zwlr_layer_shell_v1_layer layer_type;
    bool mapped;
    bool linked;
    struct wl_list link;

    struct wlr_scene_layer_surface_v1 *scene;

    struct wl_listener destroy;
    struct wl_listener map;
    struct wl_listener unmap;

    struct sycamore_output *output;
    struct sycamore_server *server;
};

void layer_map(struct sycamore_layer *layer);

void layer_unmap(struct sycamore_layer *layer);

void arrange_layers(struct sycamore_output *output);

struct sycamore_layer *sycamore_layer_create(struct sycamore_server *server,
        struct wlr_layer_surface_v1 *layer_surface);

void sycamore_layer_destroy(struct sycamore_layer *layer);

struct wlr_scene_tree *layer_get_scene_tree(struct sycamore_scene *root,
        struct sycamore_layer *layer);

#endif //SYCAMORE_LAYER_H
