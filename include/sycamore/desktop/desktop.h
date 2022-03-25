#ifndef SYCAMORE_DESKTOP_H
#define SYCAMORE_DESKTOP_H

#include "sycamore/server.h"

enum desktop_element_type {
    ELEMENT_TYPE_VIEW,
    ELEMENT_TYPE_LAYER,
};

struct wlr_surface* desktop_surface_at(struct sycamore_server *server,
                                       double lx, double ly, double *sx, double *sy);
struct sycamore_view* desktop_view_at(struct sycamore_server *server, double lx, double ly);

#endif //SYCAMORE_DESKTOP_H
