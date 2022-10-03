#ifndef SYCAMORE_SCENE_H
#define SYCAMORE_SCENE_H

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_scene.h>

struct sycamore_view;

enum scene_descriptor_type {
    SCENE_DESC_VIEW,
    SCENE_DESC_LAYER,
    SCENE_DESC_POPUP,
};

struct sycamore_scene {
    struct wlr_scene *wlr_scene;

    // tree structure:
    // - root
    //   - shell
    // 	   - background
    // 	   - bottom
    // 	   - view
    // 	   - top
    //     - overlay
    // 	 - drag_icons
    struct {
        struct wlr_scene_tree *root;
        struct wlr_scene_tree *background;
        struct wlr_scene_tree *bottom;
        struct wlr_scene_tree *view;
        struct wlr_scene_tree *top;
        struct wlr_scene_tree *overlay;
    } shell;

    struct wlr_scene_tree *drag_icons;
};

struct sycamore_scene *sycamore_scene_create(struct wlr_output_layout *layout,
        struct wlr_presentation *presentation);

void sycamore_scene_destroy(struct sycamore_scene *scene);

struct wlr_surface *find_surface(struct sycamore_scene *scene,
        double lx, double ly, double *sx, double *sy);

struct sycamore_view *find_view(struct sycamore_scene *scene, double lx, double ly);

#endif //SYCAMORE_SCENE_H