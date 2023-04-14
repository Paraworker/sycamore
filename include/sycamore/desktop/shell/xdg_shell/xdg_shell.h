#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include <wlr/types/wlr_xdg_shell.h>

typedef struct XdgShell XdgShell;

struct XdgShell {
    struct wlr_xdg_shell *wlrXdgShell;
    struct wl_listener newXdgShellSurface;
};

XdgShell *xdgShellCreate(struct wl_display *display);

void xdgShellDestroy(XdgShell *xdgShell);

#endif //SYCAMORE_XDG_SHELL_H