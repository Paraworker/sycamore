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

    wlr_scene_attach_output_layout(scene->wlr_scene, layout);
    wlr_scene_set_presentation(scene->wlr_scene, presentation);

    scene->trees.shell_background = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.shell_button = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.shell_view = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.shell_top = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.shell_overlay = wlr_scene_tree_create(&scene->wlr_scene->tree);

    return scene;
}

void sycamore_scene_destroy(struct sycamore_scene *scene) {
    if (!scene) {
        return;
    }

    free(scene);
}

struct wlr_surface *surface_under(struct sycamore_scene *scene,
        double lx, double ly, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(&scene->wlr_scene->tree.node, lx, ly, sx, sy);
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

struct sycamore_view *view_under(struct sycamore_scene *scene, double lx, double ly) {
    double sx, sy;
    struct wlr_scene_node *node = wlr_scene_node_at(&scene->wlr_scene->tree.node, lx, ly, &sx, &sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_tree *tree = node->parent;
    while (tree != NULL && tree->node.data == NULL) {
        tree = tree->node.parent;
    }

    enum scene_descriptor_type *descriptor_type = tree->node.data;
    if (*descriptor_type != SCENE_DESC_VIEW) {
        return NULL;
    }
    return tree->node.data;
}