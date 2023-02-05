#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/xdg_shell/xdg_shell.h"
#include "sycamore/desktop/shell/xdg_shell/xdg_shell_view.h"
#include "sycamore/server.h"

static void handle_new_xdg_shell_surface(struct wl_listener *listener, void *data) {
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct sycamore_xdg_shell *xdg_shell =
            wl_container_of(listener, xdg_shell, new_xdg_shell_surface);
    struct wlr_xdg_surface *xdg_surface = data;
    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_NONE) {
        return;
    }

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_scene_tree *parent_tree = xdg_surface->popup->parent->data;
        if (!parent_tree) {
            wlr_log(WLR_ERROR, "xdg_popup: parent tree is NULL");
            return;
        }

        xdg_surface->surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
        return;
    }

    // xdg_surface is toplevel
    struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;

    struct xdg_shell_view *view = xdg_shell_view_create(toplevel);
    if (!view) {
        wlr_log(WLR_ERROR, "Unable to create xdg_shell_view");
        return;
    }

    // Add to scene graph
    view->base_view.scene_tree = wlr_scene_xdg_surface_create(
            server.scene->shell.view, xdg_surface);

    struct wlr_scene_tree *scene_tree = view->base_view.scene_tree;
    scene_tree->node.data = &view->base_view;
    xdg_surface->surface->data = scene_tree;
}

struct sycamore_xdg_shell *sycamore_xdg_shell_create(struct wl_display *display) {
    struct sycamore_xdg_shell *xdg_shell = calloc(1, sizeof(struct sycamore_xdg_shell));
    if (!xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_xdg_shell");
        return NULL;
    }

    xdg_shell->wlr_xdg_shell = wlr_xdg_shell_create(display, SYCAMORE_XDG_SHELL_VERSION);
    if (!xdg_shell->wlr_xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to create wlr_xdg_shell");
        free(xdg_shell);
        return NULL;
    }

    xdg_shell->new_xdg_shell_surface.notify = handle_new_xdg_shell_surface;
    wl_signal_add(&xdg_shell->wlr_xdg_shell->events.new_surface,
                  &xdg_shell->new_xdg_shell_surface);

    return xdg_shell;
}

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell *xdg_shell) {
    if (!xdg_shell) {
        return;
    }

    wl_list_remove(&xdg_shell->new_xdg_shell_surface.link);

    free(xdg_shell);
}