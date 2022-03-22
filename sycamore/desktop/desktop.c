#include "sycamore/desktop/desktop.h"

struct sycamore_view* desktop_view_at(
        struct sycamore_server *server, double lx, double ly,
        struct wlr_surface **surface, double *sx, double *sy) {
    struct wlr_scene_node *node =
            wlr_scene_node_at(&server->scene->node, lx, ly, sx, sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }
    *surface = wlr_scene_surface_from_node(node)->surface;

    while (node != NULL && node->data == NULL) {
        node = node->parent;
    }
    return node->data;
}

