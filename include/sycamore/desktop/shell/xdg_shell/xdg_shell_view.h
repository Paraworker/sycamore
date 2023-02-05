#ifndef SYCAMORE_XDG_SHELL_VIEW_H
#define SYCAMORE_XDG_SHELL_VIEW_H

#include "sycamore/desktop/view.h"

struct xdg_shell_view {
    struct sycamore_view base_view;

    struct wlr_xdg_toplevel *xdg_toplevel;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_fullscreen;
    struct wl_listener request_maximize;
    struct wl_listener request_minimize;
};

struct xdg_shell_view *xdg_shell_view_create(struct wlr_xdg_toplevel *toplevel);

#endif //SYCAMORE_XDG_SHELL_VIEW_H