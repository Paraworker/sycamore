#ifndef SYCAMORE_VIEW_H
#define SYCAMORE_VIEW_H

#include "sycamore/server.h"
#include <wlr/types/wlr_xdg_shell.h>

enum sycamore_view_type {
    VIEW_TYPE_UNKNOWN,
    VIEW_TYPE_XDG_SHELL,
    VIEW_TYPE_XWAYLAND,
};

/* the base view */
struct sycamore_view {
    struct wl_list link;
    struct sycamore_server *server;
    struct wlr_scene_node *scene_node;
    enum sycamore_view_type type;
    int x, y;

    struct {
        void (*destroy)(struct sycamore_view *view);
        void (*set_activated)(struct sycamore_view *view, bool activated);
        void (*set_size)(struct sycamore_view *view, uint32_t width, uint32_t height);
        struct wlr_surface* (*get_wlr_surface)(struct sycamore_view *view);
        void (*get_geometry)(struct sycamore_view *view, struct wlr_box* box);
    }interface;

};

struct sycamore_xdg_shell_view {
    struct sycamore_view base_view;

    struct wlr_xdg_toplevel *xdg_toplevel;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
};

/* All type of views should call this to map */
void map_view(struct sycamore_view* view);
/* All type of views should call this to unmap */
void unmap_view(struct sycamore_view* view);
void focus_view(struct sycamore_view *view);
struct sycamore_view *desktop_view_at(struct sycamore_server *server, double lx, double ly,
        struct wlr_surface **surface, double *sx, double *sy);

#endif //SYCAMORE_VIEW_H
