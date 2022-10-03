#ifndef SYCAMORE_SERVER_H
#define SYCAMORE_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include "sycamore/desktop/shell/layer_shell.h"
#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/input/keybinding.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"

// The global server instance.
extern struct sycamore_server server;

struct sycamore_server {
    struct wl_display *wl_display;

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_compositor *compositor;
    struct wlr_output_layout *output_layout;
    struct wlr_presentation *presentation;

    struct sycamore_scene *scene;
    struct sycamore_seat *seat;
    struct sycamore_xdg_shell *xdg_shell;
    struct sycamore_layer_shell *layer_shell;
    struct sycamore_keybinding_manager *keybinding_manager;

    struct wl_listener backend_new_input;
    struct wl_listener backend_new_output;
    struct wl_listener output_layout_change;

    struct wl_list all_outputs;
    struct wl_list mapped_views;
    struct view_ptr focused_view;

    const char *socket;
};

bool server_init();

bool server_start();

void server_run();

void server_fini();

#endif //SYCAMORE_SERVER_H