#ifndef SYCAMORE_LAYER_H
#define SYCAMORE_LAYER_H

#include <wlr/types/wlr_layer_shell_v1.h>
#include "sycamore/output/scene.h"

#define LAYERS_ALL 4

typedef struct Layer  Layer;
typedef struct Output Output;

struct Layer {
    SceneDescriptorType sceneDesc;    //must be first
    struct wlr_layer_surface_v1 *layerSurface;
    struct wlr_scene_layer_surface_v1 *scene;

    bool mapped;

    bool linked;
    struct wl_list link;

    struct wl_listener destroy;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener surfaceCommit;

    Output *output;
};

void layerMap(Layer *layer);

void layerUnmap(Layer *layer);

Layer *layerCreate(struct wlr_layer_surface_v1 *layerSurface);

void layerDestroy(Layer *layer);

void arrangeLayers(Output *output);

struct wlr_scene_tree *layerGetSceneTree(Scene *scene, enum zwlr_layer_shell_v1_layer type);

#endif //SYCAMORE_LAYER_H