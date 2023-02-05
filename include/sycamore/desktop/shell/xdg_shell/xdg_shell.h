#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include <wlr/types/wlr_xdg_shell.h>

#define SYCAMORE_XDG_SHELL_VERSION 3

struct sycamore_xdg_shell {
    struct wlr_xdg_shell *wlr_xdg_shell;
    struct wl_listener new_xdg_shell_surface;
};

struct sycamore_xdg_shell *sycamore_xdg_shell_create(struct wl_display *display);

void sycamore_xdg_shell_destroy(struct sycamore_xdg_shell *xdg_shell);

#endif //SYCAMORE_XDG_SHELL_H