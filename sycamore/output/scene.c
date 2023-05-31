#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include "sycamore/output/scene.h"

Scene *sceneCreate(struct wlr_output_layout *layout,
        struct wlr_presentation *presentation, struct wlr_linux_dmabuf_v1 *dmabuf) {
    Scene *scene = calloc(1, sizeof(Scene));
    if (!scene) {
        wlr_log(WLR_ERROR, "Unable to allocate Scene");
        return NULL;
    }

    scene->wlrScene = wlr_scene_create();
    if (!scene->wlrScene) {
        wlr_log(WLR_ERROR, "Unable to create wlrScene");
        free(scene);
        return NULL;
    }

    scene->sceneDesc = SCENE_DESC_ROOT;
    scene->wlrScene->tree.node.data = scene;

    // Create trees
    scene->shell.root = wlr_scene_tree_create(&scene->wlrScene->tree);
    struct wlr_scene_tree *shellRoot = scene->shell.root;

    scene->shell.background = wlr_scene_tree_create(shellRoot);
    scene->shell.bottom     = wlr_scene_tree_create(shellRoot);
    scene->shell.view       = wlr_scene_tree_create(shellRoot);
    scene->shell.top        = wlr_scene_tree_create(shellRoot);
    scene->shell.overlay    = wlr_scene_tree_create(shellRoot);

    scene->dragIcons = wlr_scene_tree_create(&scene->wlrScene->tree);

    wlr_scene_attach_output_layout(scene->wlrScene, layout);
    wlr_scene_set_presentation(scene->wlrScene, presentation);
    wlr_scene_set_linux_dmabuf_v1(scene->wlrScene, dmabuf);

    return scene;
}

void sceneDestroy(Scene *scene) {
    if (!scene) {
        return;
    }

    if (scene->wlrScene) {
        wlr_scene_node_destroy(&scene->wlrScene->tree.node);
    }

    free(scene);
}

struct wlr_surface *getSurfaceFromNode(struct wlr_scene_node *node) {
    if (!node || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_surface *sceneSurface =
            wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node));
    if (!sceneSurface) {
        return NULL;
    }

    return sceneSurface->surface;
}

View *getViewFromNode(struct wlr_scene_node *node) {
    if (!node) {
        return NULL;
    }

    struct wlr_scene_tree *tree;
    if (node->type == WLR_SCENE_NODE_BUFFER) {
        tree = node->parent;
    } else if (node->type == WLR_SCENE_NODE_TREE) {
        tree = wlr_scene_tree_from_node(node);
    } else {
        return NULL;
    }

    while (!tree->node.data) {
        tree = tree->node.parent;
    }

    if (*(SceneDescriptorType*)(tree->node.data) != SCENE_DESC_VIEW) {
        return NULL;
    }

    return tree->node.data;
}