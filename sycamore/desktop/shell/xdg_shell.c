#include <stdlib.h>
#include <wlr/util/log.h>

#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"

void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct sycamore_xdg_toplevel_view* view =
            wl_container_of(listener, view, map);
    sycamore_view_map(&view->base_view);
}

void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct sycamore_xdg_toplevel_view *view = wl_container_of(listener, view, unmap);
    sycamore_view_unmap(&view->base_view);
}

void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct sycamore_xdg_toplevel_view *view = wl_container_of(listener, view, request_move);
    set_interactive(&view->base_view, CURSOR_MODE_MOVE, 0);
}

void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct sycamore_xdg_toplevel_view *view = wl_container_of(listener, view, request_resize);
    set_interactive(&view->base_view, CURSOR_MODE_RESIZE, event->edges);
}

void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
    struct sycamore_xdg_toplevel_view *view = wl_container_of(listener, view, request_resize);
    /* to do */
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    /* Called when the surface is destroyed and should never be shown again. */
    struct sycamore_xdg_toplevel_view *view = wl_container_of(listener, view, destroy);

    view->base_view.interface.destroy(&view->base_view);
}

/* sycamore_xdg_toplevel_view interface */
void sycamore_xdg_toplevel_view_destroy(struct sycamore_view* view) {
    if (!view) {
        return;
    }

    struct sycamore_xdg_toplevel_view* xdg_toplevel_view =
            wl_container_of(view, xdg_toplevel_view, base_view);

    wl_list_remove(&xdg_toplevel_view->map.link);
    wl_list_remove(&xdg_toplevel_view->unmap.link);
    wl_list_remove(&xdg_toplevel_view->destroy.link);
    wl_list_remove(&xdg_toplevel_view->request_move.link);
    wl_list_remove(&xdg_toplevel_view->request_resize.link);
    wl_list_remove(&xdg_toplevel_view->request_maximize.link);

    free(xdg_toplevel_view);
}

/* sycamore_xdg_toplevel_view interface */
void sycamore_xdg_toplevel_view_set_activated(struct sycamore_view* view, bool activated) {
    struct sycamore_xdg_toplevel_view* xdg_toplevel_view =
            wl_container_of(view, xdg_toplevel_view, base_view);

    wlr_xdg_toplevel_set_activated(xdg_toplevel_view->xdg_toplevel, activated);
}

/* sycamore_xdg_toplevel_view interface */
void sycamore_xdg_toplevel_view_set_size(struct sycamore_view* view, uint32_t width, uint32_t height) {
    struct sycamore_xdg_toplevel_view* xdg_toplevel_view =
            wl_container_of(view, xdg_toplevel_view, base_view);

    wlr_xdg_toplevel_set_size(xdg_toplevel_view->xdg_toplevel,
                              width, height);
}

/* sycamore_xdg_toplevel_view interface */
struct wlr_surface* sycamore_xdg_toplevel_view_get_wlr_surface(struct sycamore_view* view) {
    struct sycamore_xdg_toplevel_view* xdg_toplevel_view =
            wl_container_of(view, xdg_toplevel_view, base_view);
    return xdg_toplevel_view->xdg_toplevel->base->surface;
}

/* sycamore_xdg_toplevel_view interface */
void sycamore_xdg_toplevel_view_get_geometry(struct sycamore_view* view, struct wlr_box* box) {
    struct sycamore_xdg_toplevel_view* xdg_toplevel_view =
            wl_container_of(view, xdg_toplevel_view, base_view);

    wlr_xdg_surface_get_geometry(xdg_toplevel_view->xdg_toplevel->base, box);
}

struct sycamore_xdg_toplevel_view* sycamore_xdg_toplevel_view_create(struct sycamore_server* server,
        struct wlr_xdg_toplevel* toplevel) {
    struct sycamore_xdg_toplevel_view* view =
            calloc(1, sizeof(struct sycamore_xdg_toplevel_view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate sycamore_xdg_toplevel_view");
        return NULL;
    }

    view->base_view.type = VIEW_TYPE_XDG_TOPLEVEL;
    view->xdg_toplevel = toplevel;
    view->base_view.server = server;
    view->base_view.scene_node = wlr_scene_xdg_surface_create(
            &server->scene->node, toplevel->base);
    view->base_view.scene_node->data = &view->base_view;
    view->xdg_toplevel->base->data = view->base_view.scene_node;

    /* Listen to the various events it can emit */
    view->map.notify = xdg_toplevel_map;
    wl_signal_add(&toplevel->base->events.map, &view->map);
    view->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&toplevel->base->events.unmap, &view->unmap);
    view->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&toplevel->base->events.destroy, &view->destroy);

    /* cotd */
    view->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    view->request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);

    /* Bind interface */
    view->base_view.interface.destroy = sycamore_xdg_toplevel_view_destroy;
    view->base_view.interface.set_activated = sycamore_xdg_toplevel_view_set_activated;
    view->base_view.interface.set_size = sycamore_xdg_toplevel_view_set_size;
    view->base_view.interface.get_wlr_surface = sycamore_xdg_toplevel_view_get_wlr_surface;
    view->base_view.interface.get_geometry = sycamore_xdg_toplevel_view_get_geometry;

    return view;
}


void new_xdg_surface(struct wl_listener *listener, void *data) {
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct sycamore_xdg_shell *xdg_shell =
            wl_container_of(listener, xdg_shell, new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;

    /* We must add xdg popups to the scene graph so they get rendered. The
     * wlroots scene graph provides a helper for this, but to use it we must
     * provide the proper parent scene node of the xdg popup. To enable this,
     * we always set the user data field of xdg_surfaces to the corresponding
     * scene node. */
    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_xdg_surface *parent = wlr_xdg_surface_from_wlr_surface(
                xdg_surface->popup->parent);
        struct wlr_scene_node *parent_node = parent->data;
        xdg_surface->data = wlr_scene_xdg_surface_create(
                parent_node, xdg_surface);
        return;
    }

    /* Allocate a sycamore_xdg_toplevel_view for this surface */
    if (!sycamore_xdg_toplevel_view_create(xdg_shell->server, xdg_surface->toplevel)) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_xdg_toplevel_view");
        return;
    }
}

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell* xdg_shell) {
    if (!xdg_shell) {
        return;
    }

    wl_list_remove(&xdg_shell->new_xdg_surface.link);

    free(xdg_shell);
}

struct sycamore_xdg_shell* sycamore_xdg_shell_create(struct sycamore_server* server,
        struct wl_display* display) {
    struct sycamore_xdg_shell* xdg_shell = calloc(1, sizeof(struct sycamore_xdg_shell));
    if (!xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_xdg_shell");
        return NULL;
    }

    xdg_shell->server = server;

    xdg_shell->wlr_xdg_shell = wlr_xdg_shell_create(display);
    if (!xdg_shell->wlr_xdg_shell) {
        sycamore_xdg_shell_destroy(xdg_shell);
        return NULL;
    }

    xdg_shell->new_xdg_surface.notify = new_xdg_surface;
    wl_signal_add(&xdg_shell->wlr_xdg_shell->events.new_surface,
                  &xdg_shell->new_xdg_surface);

    return xdg_shell;
}




