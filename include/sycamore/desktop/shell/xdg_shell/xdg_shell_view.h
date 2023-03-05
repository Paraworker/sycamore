#ifndef SYCAMORE_XDG_SHELL_VIEW_H
#define SYCAMORE_XDG_SHELL_VIEW_H

#include "sycamore/desktop/view.h"

typedef struct XdgShellView XdgShellView;

struct XdgShellView {
    View baseView;

    struct wlr_xdg_toplevel *xdgToplevel;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener newPopup;
    struct wl_listener requestMove;
    struct wl_listener requestResize;
    struct wl_listener requestFullscreen;
    struct wl_listener requestMaximize;
    struct wl_listener requestMinimize;
};

XdgShellView *xdgShellViewCreate(struct wlr_xdg_toplevel *toplevel);

#endif //SYCAMORE_XDG_SHELL_VIEW_H