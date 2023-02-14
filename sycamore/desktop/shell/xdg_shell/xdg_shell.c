#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/xdg_shell/xdg_shell.h"
#include "sycamore/desktop/shell/xdg_shell/xdg_shell_view.h"
#include "sycamore/server.h"

static void onNewXdgShellSurface(struct wl_listener *listener, void *data) {
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    XdgShell *xdgShell = wl_container_of(listener, xdgShell, newXdgShellSurface);
    struct wlr_xdg_surface *xdgSurface = data;
    if (xdgSurface->role == WLR_XDG_SURFACE_ROLE_NONE) {
        return;
    }

    if (xdgSurface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_scene_tree *parentTree = xdgSurface->popup->parent->data;
        if (!parentTree) {
            wlr_log(WLR_ERROR, "xdg_popup: parent tree is NULL");
            return;
        }

        xdgSurface->surface->data = wlr_scene_xdg_surface_create(parentTree, xdgSurface);
        return;
    }

    // xdgSurface is toplevel
    struct wlr_xdg_toplevel *toplevel = xdgSurface->toplevel;

    XdgShellView *view = xdgShellViewCreate(toplevel);
    if (!view) {
        wlr_log(WLR_ERROR, "Unable to create XdgShellView");
        return;
    }

    // Add to scene graph
    view->baseView.sceneTree = wlr_scene_xdg_surface_create(
            server.scene->shell.view, xdgSurface);

    struct wlr_scene_tree *sceneTree = view->baseView.sceneTree;
    sceneTree->node.data = &view->baseView;
    xdgSurface->surface->data = sceneTree;
}

XdgShell *xdgShellCreate(struct wl_display *display) {
    XdgShell *xdgShell = calloc(1, sizeof(XdgShell));
    if (!xdgShell) {
        wlr_log(WLR_ERROR, "Unable to allocate XdgShell");
        return NULL;
    }

    xdgShell->wlrXdgShell = wlr_xdg_shell_create(display, SYCAMORE_XDG_SHELL_VERSION);
    if (!xdgShell->wlrXdgShell) {
        wlr_log(WLR_ERROR, "Unable to create wlrXdgShell");
        free(xdgShell);
        return NULL;
    }

    xdgShell->newXdgShellSurface.notify = onNewXdgShellSurface;
    wl_signal_add(&xdgShell->wlrXdgShell->events.new_surface,
                  &xdgShell->newXdgShellSurface);

    return xdgShell;
}

void xdgShellDestroy(XdgShell *xdgShell) {
    if (!xdgShell) {
        return;
    }

    wl_list_remove(&xdgShell->newXdgShellSurface.link);

    free(xdgShell);
}