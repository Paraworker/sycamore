#include <stdlib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/output/scene.h"
#include "sycamore/server.h"

struct sycamore_scene *sycamore_scene_create(struct wlr_output_layout *layout,
        struct wlr_presentation *presentation) {
    struct sycamore_scene *scene = calloc(1, sizeof(struct sycamore_scene));
    if (!scene) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_scene");
        return NULL;
    }

    scene->wlr_scene = wlr_scene_create();
    if (!scene->wlr_scene) {
        wlr_log(WLR_ERROR, "Unable to create wlr_scene");
        free(scene);
        return NULL;
    }

    scene->scene_descriptor = SCENE_DESC_ROOT;
    scene->wlr_scene->tree.node.data = scene;

    // Create trees
    scene->shell.root = wlr_scene_tree_create(&scene->wlr_scene->tree);
    struct wlr_scene_tree *shell_root = scene->shell.root;

    scene->shell.background = wlr_scene_tree_create(shell_root);
    scene->shell.bottom = wlr_scene_tree_create(shell_root);
    scene->shell.view = wlr_scene_tree_create(shell_root);
    scene->shell.top = wlr_scene_tree_create(shell_root);
    scene->shell.overlay = wlr_scene_tree_create(shell_root);

    scene->drag_icons = wlr_scene_tree_create(&scene->wlr_scene->tree);

    wlr_scene_attach_output_layout(scene->wlr_scene, layout);
    wlr_scene_set_presentation(scene->wlr_scene, presentation);

    return scene;
}

void sycamore_scene_destroy(struct sycamore_scene *scene) {
    if (!scene) {
        return;
    }

    if (scene->wlr_scene) {
        wlr_scene_node_destroy(&scene->wlr_scene->tree.node);
    }

    free(scene);
}

struct wlr_surface *find_surface_by_node(struct wlr_scene_node *node) {
    if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_surface *scene_surface =
            wlr_scene_surface_from_buffer(wlr_scene_buffer_from_node(node));
    if (!scene_surface) {
        return NULL;
    }

    return scene_surface->surface;
}

struct sycamore_view *find_view_by_node(struct wlr_scene_node *node) {
    if (!node) {
        return NULL;
    }

    struct wlr_scene_tree *tree;
    if (node->type == WLR_SCENE_NODE_BUFFER) {
        tree = node->parent;
    } else if (node->type == WLR_SCENE_NODE_TREE) {
        tree = wl_container_of(node, tree, node);
    } else {
        return NULL;
    }

    while (!tree->node.data) {
        tree = tree->node.parent;
    }

    enum scene_descriptor_type *descriptor_type = tree->node.data;
    if (*descriptor_type != SCENE_DESC_VIEW) {
        return NULL;
    }

    return tree->node.data;
}