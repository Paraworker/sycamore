#ifndef SYCAMORE_POPUP_H
#define SYCAMORE_POPUP_H

#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "sycamore/output/scene.h"

typedef struct Layer Layer;
typedef struct Popup Popup;
typedef struct View  View;

struct Popup {
    SceneDescriptorType  sceneDesc; // must be first
    struct wlr_xdg_popup *wlrPopup;

    union {
        void  *owner;
        View  *view;
        Layer *layer;
    };

    struct wlr_scene_tree *sceneTree;
    struct wlr_scene_tree *surfaceTree;

    struct wl_listener sceneTreeDestroy;
    struct wl_listener destroy;
    struct wl_listener newPopup;
};

Popup *popupCreate(struct wlr_xdg_popup *wlrPopup, struct wlr_scene_tree *parent, void *owner);

#endif //SYCAMORE_POPUP_H