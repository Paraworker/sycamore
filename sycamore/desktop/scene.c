#include "sycamore/desktop/scene.h"
#include "sycamore/desktop/view.h"
#include <stdlib.h>
#include <wlr/util/log.h>

struct sycamore_scene *sycamore_scene_create(struct sycamore_server *server,
        struct wlr_output_layout *output_layout,
                struct wlr_presentation* presentation) {
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

    scene->trees.shell_background = wlr_scene_tree_create(&scene->wlr_scene->node);
    scene->trees.shell_button = wlr_scene_tree_create(&scene->wlr_scene->node);
    scene->trees.shell_view = wlr_scene_tree_create(&scene->wlr_scene->node);
    scene->trees.shell_top = wlr_scene_tree_create(&scene->wlr_scene->node);
    scene->trees.shell_overlay = wlr_scene_tree_create(&scene->wlr_scene->node);

    scene->server = server;
    wlr_scene_attach_output_layout(scene->wlr_scene, output_layout);
    wlr_scene_set_presentation(scene->wlr_scene, presentation);

    return scene;
}

void sycamore_scene_destroy(struct sycamore_scene* scene) {
    if (!scene) {
        return;
    }

    free(scene);
}

struct wlr_surface* desktop_surface_at(struct sycamore_scene *scene,
        double lx, double ly, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(&scene->wlr_scene->node, lx, ly, sx, sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }
    return wlr_scene_surface_from_node(node)->surface;
}

struct sycamore_view* desktop_view_at(struct sycamore_scene *scene, double lx, double ly) {
    double sx, sy;
    struct wlr_scene_node *node = wlr_scene_node_at(&scene->wlr_scene->node, lx, ly, &sx, &sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }

    while (node != NULL && node->data == NULL) {
        node = node->parent;
    }

    enum scene_descriptor_type *descriptor_type = node->data;
    if (*descriptor_type != SCENE_DESC_VIEW) {
        return NULL;
    }
    return node->data;
}

