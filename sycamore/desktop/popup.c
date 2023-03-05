#include <stdlib.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/popup.h"

static void onSceneTreeDestroy(struct wl_listener *listener, void *data) {
    Popup *popup = wl_container_of(listener, popup, sceneTreeDestroy);

    wl_list_remove(&popup->sceneTreeDestroy.link);
    wl_list_remove(&popup->destroy.link);
    wl_list_remove(&popup->newPopup.link);

    free(popup);
}

static void onDestroy(struct wl_listener *listener, void *data) {
    Popup *popup = wl_container_of(listener, popup, destroy);
    wlr_scene_node_destroy(&popup->sceneTree->node);
}

static void onNewPopup(struct wl_listener *listener, void *data) {
    Popup *popup = wl_container_of(listener, popup, newPopup);
    struct wlr_xdg_popup *wlrPopup = data;

    popupCreate(wlrPopup, popup->surfaceTree, popup->owner);
}

Popup *popupCreate(struct wlr_xdg_popup *wlrPopup, struct wlr_scene_tree *parent, void *owner) {
    Popup *popup = calloc(1, sizeof(Popup));
    if (!popup) {
        wlr_log(WLR_ERROR, "Unable to allocate popup");
        return nullptr;
    }

    popup->sceneDesc = SCENE_DESC_POPUP;
    popup->wlrPopup  = wlrPopup;
    popup->owner     = owner;

    popup->sceneTree   = wlr_scene_tree_create(parent);
    popup->surfaceTree = wlr_scene_xdg_surface_create(popup->sceneTree, wlrPopup->base);
    popup->sceneTree->node.data = popup;

    popup->newPopup.notify = onNewPopup;
    wl_signal_add(&wlrPopup->base->events.new_popup, &popup->newPopup);
    popup->destroy.notify = onDestroy;
    wl_signal_add(&wlrPopup->base->events.destroy, &popup->destroy);
    popup->sceneTreeDestroy.notify = onSceneTreeDestroy;
    wl_signal_add(&popup->sceneTree->node.events.destroy, &popup->sceneTreeDestroy);

    return popup;
}