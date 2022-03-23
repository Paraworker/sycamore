#ifndef SYCAMORE_SERVER_H
#define SYCAMORE_SERVER_H

#include <stdbool.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_presentation_time.h>

#include "sycamore/input/seat.h"
#include "sycamore/desktop/view.h"
#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/desktop/shell/layer_shell.h"
#include "sycamore/input/keybinding.h"

struct sycamore_server {
    struct wl_display *wl_display;

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wl_list all_outputs;
    struct wl_listener backend_new_output;
    struct wl_listener backend_new_input;

    struct wlr_compositor* compositor;
    struct wlr_scene *scene;
    struct wlr_presentation* presentation;

    struct wlr_output_layout *output_layout;
    struct wl_listener output_layout_change;

    struct sycamore_seat* seat;
    struct sycamore_xdg_shell* xdg_shell;
    struct sycamore_layer_shell* layer_shell;
    struct sycamore_keybinding_manager* keybinding_manager;

    struct wl_list mapped_views;
    struct sycamore_view* desktop_focused_view;

    const char *socket;
};

struct sycamore_server* server_create();
bool server_start(struct sycamore_server* server);
void server_run(struct sycamore_server* server);
void server_destroy(struct sycamore_server* server);


#endif //SYCAMORE_SERVER_H
