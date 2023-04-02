#include <stdlib.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/xdg_shell/xdg_shell_view.h"
#include "sycamore/input/seat.h"
#include "sycamore/server.h"

static void onXdgShellViewRequestMove(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    XdgShellView *view = wl_container_of(listener, view, requestMove);
    View *base = &view->baseView;
    seatopSetPointerMove(server.seat, base);
}

static void onXdgShellViewRequestResize(struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    XdgShellView *view = wl_container_of(listener, view, requestResize);
    View *base = &view->baseView;
    struct wlr_xdg_toplevel_resize_event *event = data;
    seatopSetPointerResize(server.seat, base, event->edges);
}

static void onXdgShellViewRequestFullscreen(struct wl_listener *listener, void *data) {
    XdgShellView *view = wl_container_of(listener, view, requestFullscreen);
    View *base = &view->baseView;

    struct wlr_xdg_toplevel_requested *requested = &view->xdgToplevel->requested;

    if (!requested->fullscreen) {
        viewSetFullscreen(base, NULL, false);
        return;
    }

    // If there is an output provided, try to satisfy it
    struct wlr_output *output = requested->fullscreen_output;
    if (output && output->data) {
        viewSetFullscreen(base, output->data, true);
        return;
    }

    viewSetFullscreen(base, viewGetOutput(base), true);
}

static void onXdgShellViewRequestMaximize(struct wl_listener *listener, void *data) {
    XdgShellView *view = wl_container_of(listener, view, requestMaximize);
    View *base = &view->baseView;

    if (!view->xdgToplevel->requested.maximized) {
        viewSetMaximized(base, NULL, false);
        return;
    }

    viewSetMaximized(base, viewGetOutput(base), true);
}

static void onXdgShellViewRequestMinimize(struct wl_listener *listener, void *data) {
    XdgShellView *view = wl_container_of(listener, view, requestMinimize);

    wlr_xdg_surface_schedule_configure(view->xdgToplevel->base);
}

static void onXdgShellViewDestroy(struct wl_listener *listener, void *data) {
    /* Called when the surface is destroyed and should never be shown again. */
    XdgShellView *view = wl_container_of(listener, view, destroy);

    viewDestroy(&view->baseView);
}

static void onXdgShellViewMap(struct wl_listener *listener, void *data) {
    /* Called when the surface is mapped, or ready to display on-screen. */
    XdgShellView *view = wl_container_of(listener, view, map);
    struct wlr_xdg_toplevel_requested *requested = &view->xdgToplevel->requested;

    viewMap(&view->baseView,
            requested->fullscreen_output,
            requested->maximized,
            requested->fullscreen);
}

static void onXdgShellViewUnmap(struct wl_listener *listener, void *data) {
    /* Called when the surface is unmapped, and should no longer be shown. */
    XdgShellView *view = wl_container_of(listener, view, unmap);

    viewUnmap(&view->baseView);
}

// view interface
static void xdgShellViewDestroy(View *view) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);

    wl_list_remove(&xdgShellView->destroy.link);
    wl_list_remove(&xdgShellView->map.link);
    wl_list_remove(&xdgShellView->unmap.link);

    free(xdgShellView);
}

// view interface
static void xdgShellViewMap(View *view) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    struct wlr_xdg_toplevel *toplevel = xdgShellView->xdgToplevel;

    xdgShellView->requestMove.notify = onXdgShellViewRequestMove;
    wl_signal_add(&toplevel->events.request_move,
                  &xdgShellView->requestMove);
    xdgShellView->requestResize.notify = onXdgShellViewRequestResize;
    wl_signal_add(&toplevel->events.request_resize,
                  &xdgShellView->requestResize);
    xdgShellView->requestFullscreen.notify = onXdgShellViewRequestFullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen,
                  &xdgShellView->requestFullscreen);
    xdgShellView->requestMaximize.notify = onXdgShellViewRequestMaximize;
    wl_signal_add(&toplevel->events.request_maximize,
                  &xdgShellView->requestMaximize);
    xdgShellView->requestMinimize.notify = onXdgShellViewRequestMinimize;
    wl_signal_add(&toplevel->events.request_minimize,
                  &xdgShellView->requestMinimize);
}

// view interface
static void xdgShellViewUnmap(View *view) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);

    wl_list_remove(&xdgShellView->requestMove.link);
    wl_list_remove(&xdgShellView->requestResize.link);
    wl_list_remove(&xdgShellView->requestFullscreen.link);
    wl_list_remove(&xdgShellView->requestMaximize.link);
    wl_list_remove(&xdgShellView->requestMinimize.link);
}

// view interface
static void xdgShellViewSetActivated(View *view, bool activated) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_set_activated(xdgShellView->xdgToplevel, activated);
}

// view interface
static void xdgShellViewSetSize(View *view, uint32_t width, uint32_t height) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_set_size(xdgShellView->xdgToplevel, width, height);
}

// view interface
static void xdgShellViewSetFullscreen(View *view, bool fullscreen) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_set_fullscreen(xdgShellView->xdgToplevel, fullscreen);
}

// view interface
static void xdgShellViewSetMaximized(View *view, bool maximized) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_set_maximized(xdgShellView->xdgToplevel, maximized);
}

// view interface
static void xdgShellViewSetResizing(View *view, bool resizing) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_set_resizing(xdgShellView->xdgToplevel, resizing);
}

// view interface
static void xdgShellViewGetGeometry(View *view, struct wlr_box *box) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_surface_get_geometry(xdgShellView->xdgToplevel->base, box);
}

// view interface
static void xdgShellViewClose(View *view) {
    XdgShellView *xdgShellView = wl_container_of(view, xdgShellView, baseView);
    wlr_xdg_toplevel_send_close(xdgShellView->xdgToplevel);
}

static const ViewInterface interface = {
        .destroy       = xdgShellViewDestroy,
        .map           = xdgShellViewMap,
        .unmap         = xdgShellViewUnmap,
        .setActivated  = xdgShellViewSetActivated,
        .setSize       = xdgShellViewSetSize,
        .setFullscreen = xdgShellViewSetFullscreen,
        .setMaximized  = xdgShellViewSetMaximized,
        .setResizing   = xdgShellViewSetResizing,
        .getGeometry   = xdgShellViewGetGeometry,
        .close         = xdgShellViewClose,
};

XdgShellView *xdgShellViewCreate(struct wlr_xdg_toplevel *toplevel) {
    XdgShellView *view = calloc(1, sizeof(XdgShellView));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate XdgShellView");
        return NULL;
    }

    viewInit(&view->baseView, VIEW_TYPE_XDG_SHELL,
             toplevel->base->surface, &interface);

    view->xdgToplevel = toplevel;

    view->map.notify = onXdgShellViewMap;
    wl_signal_add(&toplevel->base->events.map, &view->map);
    view->unmap.notify = onXdgShellViewUnmap;
    wl_signal_add(&toplevel->base->events.unmap, &view->unmap);
    view->destroy.notify = onXdgShellViewDestroy;
    wl_signal_add(&toplevel->base->events.destroy, &view->destroy);

    return view;
}