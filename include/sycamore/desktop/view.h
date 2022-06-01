#ifndef SYCAMORE_VIEW_H
#define SYCAMORE_VIEW_H

#include <stdbool.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include "sycamore/output/scene.h"

struct sycamore_view;
struct sycamore_output;
struct sycamore_server;

enum sycamore_view_type {
    VIEW_TYPE_UNKNOWN,
    VIEW_TYPE_XDG_SHELL,
    VIEW_TYPE_XWAYLAND,
};

struct view_ptr {
    struct sycamore_view *view;
    struct wl_list link;    //view::ptrs
};

struct view_interface {
    void (*destroy)(struct sycamore_view *view);
    void (*map)(struct sycamore_view *view);
    void (*unmap)(struct sycamore_view *view);
    void (*set_activated)(struct sycamore_view *view, bool activated);
    void (*set_size)(struct sycamore_view *view, uint32_t width, uint32_t height);
    void (*set_fullscreen)(struct sycamore_view *view, bool fullscreen);
    void (*set_maximized)(struct sycamore_view *view, bool maximized);
    void (*set_resizing)(struct sycamore_view *view, bool resizing);
    void (*get_geometry)(struct sycamore_view *view, struct wlr_box *box);
};

/* base view */
struct sycamore_view {
    enum scene_descriptor_type scene_descriptor;    //must be first
    const struct view_interface *interface;
    struct wlr_surface *wlr_surface;
    enum sycamore_view_type view_type;
    int x, y;

    struct wlr_scene_tree *scene_tree;

    struct wl_list link;
    struct wl_list ptrs;

    bool mapped;
    bool is_maximized;
    bool is_fullscreen;

    struct wlr_box maximize_restore;
    struct wlr_box fullscreen_restore;

    struct sycamore_server *server;
};

struct sycamore_xdg_shell_view {
    struct sycamore_view base_view;

    struct wlr_xdg_toplevel *xdg_toplevel;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_fullscreen;
    struct wl_listener request_maximize;
};

void view_init(struct sycamore_view *view, struct wlr_surface *surface,
        const struct view_interface *interface, struct sycamore_server *server);

void view_map(struct sycamore_view *view,
        struct wlr_output *fullscreen_output, bool maximized, bool fullscreen);

void view_unmap(struct sycamore_view *view);

void view_destroy(struct sycamore_view *view);

void view_move_to(struct sycamore_view *view, int x, int y);

struct sycamore_output *view_get_main_output(struct sycamore_view *view);

void view_set_fullscreen(struct sycamore_view *view,
        const struct wlr_box *full_box, bool fullscreen);

void view_set_maximized(struct sycamore_view *view,
        const struct wlr_box *max_box, bool maximized);

void view_set_focus(struct sycamore_view *view);

void view_ptr_connect(struct view_ptr *ptr, struct sycamore_view *view);

void view_ptr_disconnect(struct view_ptr *ptr);

#endif //SYCAMORE_VIEW_H
