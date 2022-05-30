#include <stdlib.h>
#include <stdbool.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/desktop/view.h"
#include "sycamore/output/output.h"

static void handle_xdg_shell_view_request_move(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_move);
    seatop_begin_pointer_move(view->base_view.server->seat, &view->base_view);
}

static void handle_xdg_shell_view_request_resize(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_resize);
    seatop_begin_pointer_resize(view->base_view.server->seat, &view->base_view, event->edges);
}

static void handle_xdg_shell_view_request_fullscreen(struct wl_listener *listener, void *data) {
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_fullscreen);
    struct sycamore_view *base = &view->base_view;
    bool fullscreen = view->xdg_toplevel->requested.fullscreen;
    if (fullscreen) {
        /* If there is a fullscreen_output, satisfy it first */
        struct wlr_output *output = view->xdg_toplevel->requested.fullscreen_output;
        if (!output) {
            struct sycamore_output *sycamore_output = view_get_main_output(base);
            if (sycamore_output) {
                output = sycamore_output->wlr_output;
            }
        }

        /* output may still be NULL, but we apply it anyway */
        struct wlr_box box;
        wlr_output_layout_get_box(base->server->output_layout, output, &box);

        view_set_fullscreen(base, &box, fullscreen);
    } else {
        view_set_fullscreen(base, NULL, fullscreen);
    }
}

static void handle_xdg_shell_view_request_maximize(struct wl_listener *listener, void *data) {
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, request_maximize);
    struct sycamore_view *base = &view->base_view;
    bool maximized = view->xdg_toplevel->requested.maximized;
    if (maximized) {
        struct sycamore_output *output = view_get_main_output(base);
        if (output) {
            view_set_maximized(base, &output->usable_area, maximized);
        } else {
            struct wlr_box box;
            wlr_output_layout_get_box(base->server->output_layout, NULL, &box);
            view_set_maximized(base, &box, maximized);
        }
    } else {
        view_set_maximized(base, NULL, maximized);
    }
}

static void handle_xdg_shell_view_destroy(struct wl_listener *listener, void *data) {
    /* Called when the surface is destroyed and should never be shown again. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, destroy);

    view_destroy(&view->base_view);
}

static void handle_xdg_shell_view_map(struct wl_listener *listener, void *data) {
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, map);
    struct wlr_xdg_toplevel_requested *requested = &view->xdg_toplevel->requested;

    view_map(&view->base_view,
             requested->fullscreen_output,
             requested->maximized,
             requested->fullscreen);
}

static void handle_xdg_shell_view_unmap(struct wl_listener *listener, void *data) {
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct sycamore_xdg_shell_view *view = wl_container_of(listener, view, unmap);

    view_unmap(&view->base_view);
}

/* view interface */
static void xdg_shell_view_destroy(struct sycamore_view *view) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wl_list_remove(&xdg_shell_view->map.link);
    wl_list_remove(&xdg_shell_view->unmap.link);
    wl_list_remove(&xdg_shell_view->destroy.link);

    free(xdg_shell_view);
}

/* view interface */
static void xdg_shell_view_map(struct sycamore_view *view) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    struct wlr_xdg_toplevel *toplevel = xdg_shell_view->xdg_toplevel;

    xdg_shell_view->request_move.notify = handle_xdg_shell_view_request_move;
    wl_signal_add(&toplevel->events.request_move,
                  &xdg_shell_view->request_move);
    xdg_shell_view->request_resize.notify = handle_xdg_shell_view_request_resize;
    wl_signal_add(&toplevel->events.request_resize,
                  &xdg_shell_view->request_resize);
    xdg_shell_view->request_fullscreen.notify = handle_xdg_shell_view_request_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen,
                  &xdg_shell_view->request_fullscreen);
    xdg_shell_view->request_maximize.notify = handle_xdg_shell_view_request_maximize;
    wl_signal_add(&toplevel->events.request_maximize,
                  &xdg_shell_view->request_maximize);
}

/* view interface */
static void xdg_shell_view_unmap(struct sycamore_view *view) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wl_list_remove(&xdg_shell_view->request_move.link);
    wl_list_remove(&xdg_shell_view->request_resize.link);
    wl_list_remove(&xdg_shell_view->request_maximize.link);
    wl_list_remove(&xdg_shell_view->request_fullscreen.link);
}

/* view interface */
static void xdg_shell_view_set_activated(struct sycamore_view *view, bool activated) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_toplevel_set_activated(xdg_shell_view->xdg_toplevel, activated);
}

/* view interface */
static void xdg_shell_view_set_size(struct sycamore_view *view, uint32_t width, uint32_t height) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_toplevel_set_size(xdg_shell_view->xdg_toplevel,
                              width, height);
}

/* view interface */
static void xdg_shell_view_set_fullscreen(struct sycamore_view *view, bool fullscreen) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    wlr_xdg_toplevel_set_fullscreen(xdg_shell_view->xdg_toplevel, fullscreen);
}

/* view interface */
static void xdg_shell_view_set_maximized(struct sycamore_view *view, bool maximized) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    wlr_xdg_toplevel_set_maximized(xdg_shell_view->xdg_toplevel, maximized);
}

/* view interface */
static void xdg_shell_view_set_resizing(struct sycamore_view *view, bool resizing) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);
    wlr_xdg_toplevel_set_resizing(xdg_shell_view->xdg_toplevel, resizing);
}

/* view interface */
static void xdg_shell_view_get_geometry(struct sycamore_view *view, struct wlr_box *box) {
    struct sycamore_xdg_shell_view *xdg_shell_view =
            wl_container_of(view, xdg_shell_view, base_view);

    wlr_xdg_surface_get_geometry(xdg_shell_view->xdg_toplevel->base, box);
}

static const struct view_interface xdg_shell_view_interface = {
    .destroy = xdg_shell_view_destroy,
    .map = xdg_shell_view_map,
    .unmap = xdg_shell_view_unmap,
    .set_activated = xdg_shell_view_set_activated,
    .set_size = xdg_shell_view_set_size,
    .set_fullscreen = xdg_shell_view_set_fullscreen,
    .set_maximized = xdg_shell_view_set_maximized,
    .set_resizing = xdg_shell_view_set_resizing,
    .get_geometry = xdg_shell_view_get_geometry,
};

struct sycamore_xdg_shell_view *sycamore_xdg_shell_view_create(
        struct sycamore_server *server, struct wlr_xdg_toplevel *toplevel) {
    struct sycamore_xdg_shell_view *view =
            calloc(1, sizeof(struct sycamore_xdg_shell_view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate sycamore_xdg_shell_view");
        return NULL;
    }

    view_init(&view->base_view, toplevel->base->surface,
              &xdg_shell_view_interface, server);

    view->xdg_toplevel = toplevel;

    view->map.notify = handle_xdg_shell_view_map;
    wl_signal_add(&toplevel->base->events.map, &view->map);
    view->unmap.notify = handle_xdg_shell_view_unmap;
    wl_signal_add(&toplevel->base->events.unmap, &view->unmap);
    view->destroy.notify = handle_xdg_shell_view_destroy;
    wl_signal_add(&toplevel->base->events.destroy, &view->destroy);

    return view;
}


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
        struct wlr_surface *parent_surface = xdg_surface->popup->parent;

        struct wlr_scene_node *parent_node = NULL;
        if (wlr_surface_is_xdg_surface(parent_surface)) {
            struct wlr_xdg_surface *parent =
                    wlr_xdg_surface_from_wlr_surface(parent_surface);
            parent_node = parent->data;
        } else if (wlr_surface_is_layer_surface(parent_surface)) {
            struct wlr_layer_surface_v1 *parent =
                    wlr_layer_surface_v1_from_wlr_surface(parent_surface);
            parent_node = parent->data;
        } else {
            wlr_log(WLR_ERROR, "unknown parent surface type");
            return;
        }

        xdg_surface->data = wlr_scene_xdg_surface_create(parent_node, xdg_surface);
        return;
    }

    struct sycamore_server *server = xdg_shell->server;
    struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;

    /* Allocate a sycamore_xdg_shell_view for this surface */
    struct sycamore_xdg_shell_view *view =
            sycamore_xdg_shell_view_create(server, toplevel);
    if (!view) {
        wlr_log(WLR_ERROR, "Unable to create xdg_shell_view");
        return;
    }

    /* Add to scene graph */
    view->base_view.scene_node = wlr_scene_xdg_surface_create(
            &server->scene->trees.shell_view->node, xdg_surface);
    view->base_view.scene_node->data = &view->base_view;
    xdg_surface->data = view->base_view.scene_node;
}

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell *xdg_shell) {
    if (!xdg_shell) {
        return;
    }

    wl_list_remove(&xdg_shell->new_xdg_shell_surface.link);

    free(xdg_shell);
}

struct sycamore_xdg_shell *sycamore_xdg_shell_create(struct sycamore_server *server,
        struct wl_display *display) {
    struct sycamore_xdg_shell *xdg_shell = calloc(1, sizeof(struct sycamore_xdg_shell));
    if (!xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_xdg_shell");
        return NULL;
    }

    xdg_shell->server = server;

    xdg_shell->wlr_xdg_shell = wlr_xdg_shell_create(display, 3);
    if (!xdg_shell->wlr_xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to create_wlr_xdg_shell");
        free(xdg_shell);
        return NULL;
    }

    xdg_shell->new_xdg_shell_surface.notify = handle_new_xdg_shell_surface;
    wl_signal_add(&xdg_shell->wlr_xdg_shell->events.new_surface,
                  &xdg_shell->new_xdg_shell_surface);

    return xdg_shell;
}




