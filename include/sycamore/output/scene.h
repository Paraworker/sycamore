#ifndef SYCAMORE_SCENE_H
#define SYCAMORE_SCENE_H

#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_scene.h>

typedef struct Scene             Scene;
typedef enum SceneDescriptorType SceneDescriptorType;
typedef struct View              View;

enum SceneDescriptorType {
    SCENE_DESC_ROOT,
    SCENE_DESC_VIEW,
    SCENE_DESC_LAYER,
    SCENE_DESC_POPUP,
    SCENE_DESC_DRAG_ICON,
};

struct Scene {
    SceneDescriptorType sceneDesc; // must be first
    struct wlr_scene    *wlrScene;

    // tree structure:
    // - root
    //   - shell
    // 	   - background
    // 	   - bottom
    // 	   - view
    // 	   - top
    //     - overlay
    // 	 - dragIcons
    struct {
        struct wlr_scene_tree *root;
        struct wlr_scene_tree *background;
        struct wlr_scene_tree *bottom;
        struct wlr_scene_tree *view;
        struct wlr_scene_tree *top;
        struct wlr_scene_tree *overlay;
    } shell;

    struct wlr_scene_tree *dragIcons;
};

Scene *sceneCreate(struct wlr_output_layout *layout,
        struct wlr_presentation *presentation, struct wlr_linux_dmabuf_v1 *dmabuf);

void sceneDestroy(Scene *scene);

#define FIND_NODE(lx, ly, sx, sy) wlr_scene_node_at(&server.scene->shell.root->node, lx, ly, sx, sy)

struct wlr_surface *getSurfaceFromNode(struct wlr_scene_node *node);

View *getViewFromNode(struct wlr_scene_node *node);

struct wlr_scene_tree *sceneGetLayerTree(Scene *scene, enum zwlr_layer_shell_v1_layer type);

#endif //SYCAMORE_SCENE_H