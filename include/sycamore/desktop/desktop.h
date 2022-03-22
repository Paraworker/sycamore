#ifndef SYCAMORE_DESKTOP_H
#define SYCAMORE_DESKTOP_H

#include "sycamore/server.h"

struct sycamore_view* desktop_view_at(
        struct sycamore_server *server, double lx, double ly,
        struct wlr_surface **surface, double *sx, double *sy);

#endif //SYCAMORE_DESKTOP_H
