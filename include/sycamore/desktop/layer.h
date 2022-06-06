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
    bool mapped;
    bool linked;
    struct wl_list link;

    struct wlr_scene_layer_surface_v1 *scene;

    struct wl_listener destroy;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener surface_commit;

    struct sycamore_output *output;
    struct sycamore_server *server;
};

void layer_map(struct sycamore_layer *layer);

void layer_unmap(struct sycamore_layer *layer);

void layer_surface_commit(struct sycamore_layer *layer);

struct sycamore_layer *layer_create(struct sycamore_server *server,
        struct wlr_layer_surface_v1 *layer_surface);

void layer_destroy(struct sycamore_layer *layer);

void arrange_layers(struct sycamore_output *output);

struct wlr_scene_tree *layer_get_scene_tree(struct sycamore_scene *root,
        enum zwlr_layer_shell_v1_layer type);

#endif //SYCAMORE_LAYER_H
