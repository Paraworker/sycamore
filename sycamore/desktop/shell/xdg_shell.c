#include <stdlib.h>
#include <wlr/util/log.h>

#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/desktop/view.h"
#include "sycamore/input/cursor.h"

void handle_xdg_shell_view_map(struct wl_listener *listener, void *data) {
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct sycamore_xdg_shell_view* view =
            wl_container_of(listener, view, map);
    view_map(&view->base_view, view->xdg_toplevel->requested.fullscreen_output,
             view->xdg_toplevel->requested.maximized, view->xdg_toplevel->requested.fullscreen);
}

void handle_xdg_shell_view_unmap(struct wl_listener *listener, void *data) {
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, unmap);
    view_unmap(&view->base_view);
}

void handle_xdg_shell_view_request_move(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_move);
    set_interactive(&view->base_view, CURSOR_MODE_MOVE, 0);
}

void handle_xdg_shell_view_request_resize(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_resize);
    set_interactive(&view->base_view, CURSOR_MODE_RESIZE, event->edges);
}

void handle_xdg_shell_view_request_fullscreen(struct wl_listener *listener, void *data) {
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_fullscreen);
    view_set_fullscreen(&view->base_view, view->xdg_toplevel->requested.fullscreen_output,
                        view->xdg_toplevel->requested.fullscreen);
}

void handle_xdg_shell_view_request_maximized(struct wl_listener *listener, void *data) {
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_maximize);
    view_set_maximized(&view->base_view, view->xdg_toplevel->requested.maximized);
}

void handle_xdg_shell_view_destroy(struct wl_listener *listener, void *data) {
    /* Called when the surface is destroyed and should never be shown again. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, destroy);

    view->base_view.interface->destroy(&view->base_view);
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_destroy(struct sycamore_view* view) {
    if (!view) {
        return;
    }

    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wl_list_remove(&xdg_shell_view->map.link);
    wl_list_remove(&xdg_shell_view->unmap.link);
    wl_list_remove(&xdg_shell_view->destroy.link);
    wl_list_remove(&xdg_shell_view->request_move.link);
    wl_list_remove(&xdg_shell_view->request_resize.link);
    wl_list_remove(&xdg_shell_view->request_maximize.link);

    free(xdg_shell_view);
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_set_activated(struct sycamore_view* view, bool activated) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_toplevel_set_activated(xdg_shell_view->xdg_toplevel, activated);
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_set_size(struct sycamore_view* view, uint32_t width, uint32_t height) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_toplevel_set_size(xdg_shell_view->xdg_toplevel,
                              width, height);
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_set_fullscreen(struct sycamore_view* view, bool fullscreen) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    wlr_xdg_toplevel_set_fullscreen(xdg_shell_view->xdg_toplevel, fullscreen);
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_set_maximized(struct sycamore_view* view, bool maximized) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    wlr_xdg_toplevel_set_maximized(xdg_shell_view->xdg_toplevel, maximized);
}

/* sycamore_xdg_shell_view interface */
struct wlr_surface* sycamore_xdg_shell_view_get_wlr_surface(struct sycamore_view* view) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    return xdg_shell_view->xdg_toplevel->base->surface;
}

/* sycamore_xdg_shell_view interface */
void sycamore_xdg_shell_view_get_geometry(struct sycamore_view* view, struct wlr_box* box) {
    struct sycamore_xdg_shell_view* xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_surface_get_geometry(xdg_shell_view->xdg_toplevel->base, box);
}

static const struct sycamore_view_interface xdg_shell_view_interface = {
    .destroy = sycamore_xdg_shell_view_destroy,
    .set_activated = sycamore_xdg_shell_view_set_activated,
    .set_size = sycamore_xdg_shell_view_set_size,
    .set_fullscreen = sycamore_xdg_shell_view_set_fullscreen,
    .set_maximized = sycamore_xdg_shell_view_set_maximized,
    .get_wlr_surface =  sycamore_xdg_shell_view_get_wlr_surface,
    .get_geometry = sycamore_xdg_shell_view_get_geometry,
};

struct sycamore_xdg_shell_view* sycamore_xdg_shell_view_create(struct sycamore_server* server,
                                                               struct wlr_xdg_toplevel* toplevel) {
    struct sycamore_xdg_shell_view* view =
            calloc(1, sizeof(struct sycamore_xdg_shell_view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate sycamore_xdg_shell_view");
        return NULL;
    }

    view->base_view.type = VIEW_TYPE_XDG_SHELL;
    view->xdg_toplevel = toplevel;
    view->base_view.interface = &xdg_shell_view_interface;
    view->base_view.is_fullscreen = false;
    view->base_view.is_maximized = false;
    view->base_view.server = server;
    view->base_view.scene_node = wlr_scene_xdg_surface_create(
            &server->scene->node, toplevel->base);
    view->base_view.scene_node->data = &view->base_view;
    view->xdg_toplevel->base->data = view->base_view.scene_node;

    /* Listen to the various events it can emit */
    view->map.notify = handle_xdg_shell_view_map;
    wl_signal_add(&toplevel->base->events.map, &view->map);
    view->unmap.notify = handle_xdg_shell_view_unmap;
    wl_signal_add(&toplevel->base->events.unmap, &view->unmap);
    view->destroy.notify = handle_xdg_shell_view_destroy;
    wl_signal_add(&toplevel->base->events.destroy, &view->destroy);

    /* cotd */
    view->request_move.notify = handle_xdg_shell_view_request_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = handle_xdg_shell_view_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    view->request_fullscreen.notify = handle_xdg_shell_view_request_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);
    view->request_maximize.notify = handle_xdg_shell_view_request_maximized;
    wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);

    return view;
}


static void handle_new_xdg_shell_surface(struct wl_listener *listener, void *data) {
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct sycamore_xdg_shell *xdg_shell =
            wl_container_of(listener, xdg_shell, new_xdg_shell_surface);
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

    /* Allocate a sycamore_xdg_shell_view for this surface */
    if (!sycamore_xdg_shell_view_create(xdg_shell->server, xdg_surface->toplevel)) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_xdg_shell_view");
        return;
    }
}

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell* xdg_shell) {
    if (!xdg_shell) {
        return;
    }

    wl_list_remove(&xdg_shell->new_xdg_shell_surface.link);

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
        wlr_log(WLR_ERROR, "Unable to create_wlr_xdg_shell");
        sycamore_xdg_shell_destroy(xdg_shell);
        return NULL;
    }

    xdg_shell->new_xdg_shell_surface.notify = handle_new_xdg_shell_surface;
    wl_signal_add(&xdg_shell->wlr_xdg_shell->events.new_surface,
                  &xdg_shell->new_xdg_shell_surface);

    return xdg_shell;
}




