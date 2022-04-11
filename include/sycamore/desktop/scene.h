#ifndef SYCAMORE_SCENE_H
#define SYCAMORE_SCENE_H

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_scene.h>
#include "sycamore/server.h"

enum scene_descriptor_type {
    SCENE_DESC_VIEW,
    SCENE_DESC_LAYER,
    SCENE_DESC_POPUP,
};

struct sycamore_scene {
    struct wlr_scene *wlr_scene;

    struct {
        struct wlr_scene_tree *shell_background;
        struct wlr_scene_tree *shell_button;
        struct wlr_scene_tree *shell_view;
        struct wlr_scene_tree *shell_top;
        struct wlr_scene_tree *shell_overlay;
    } trees;

    struct sycamore_server *server;
};

struct sycamore_scene *sycamore_scene_create(struct sycamore_server *server,
        struct wlr_output_layout *output_layout, struct wlr_presentation* presentation);

void sycamore_scene_destroy(struct sycamore_scene* scene);

struct wlr_surface *surface_under(struct sycamore_scene *scene,
        double lx, double ly, double *sx, double *sy);

struct sycamore_view* view_under(struct sycamore_scene *scene, double lx, double ly);

#endif //SYCAMORE_SCENE_H
