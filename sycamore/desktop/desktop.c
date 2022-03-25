#include "sycamore/desktop/desktop.h"

struct wlr_surface* desktop_surface_at(struct sycamore_server *server,
        double lx, double ly, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->node, lx, ly, sx, sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }
    return wlr_scene_surface_from_node(node)->surface;
}

struct sycamore_view* desktop_view_at(struct sycamore_server *server, double lx, double ly) {
    double sx, sy;
    struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->node, lx, ly, &sx, &sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }

    while (node != NULL && node->data == NULL) {
        node = node->parent;
    }

    enum desktop_element_type* type = node->data;
    if (*type != ELEMENT_TYPE_VIEW) {
        return NULL;
    }
    return node->data;
}

