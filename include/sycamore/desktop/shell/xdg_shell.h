#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include <wlr/types/wlr_xdg_shell.h>
#include "sycamore/server.h"

struct sycamore_xdg_shell {
    struct wlr_xdg_shell *wlr_xdg_shell;

    struct wl_listener new_xdg_shell_surface;

    struct sycamore_server *server;
};

struct sycamore_xdg_shell *sycamore_xdg_shell_create(struct sycamore_server *server,
        struct wl_display *display);

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell *xdg_shell);

struct sycamore_xdg_shell_view *sycamore_xdg_shell_view_create(struct sycamore_server *server,
        struct wlr_xdg_toplevel *toplevel);

#endif //SYCAMORE_XDG_SHELL_H