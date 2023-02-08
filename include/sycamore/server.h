#ifndef SYCAMORE_SERVER_H
#define SYCAMORE_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include "sycamore/desktop/view.h"

struct sycamore_scene;
struct sycamore_seat;
struct sycamore_xdg_shell;
struct sycamore_layer_shell;
struct sycamore_keybinding_manager;

// The global server instance.
extern struct sycamore_server server;

struct sycamore_server {
    struct wl_display *wl_display;

    struct wlr_backend *backend;
    struct wlr_session *session;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_compositor *compositor;

    struct wlr_linux_dmabuf_v1 *dmabuf;

    struct wlr_presentation *presentation;

    struct wlr_output_layout *output_layout;

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

/**
 * @brief Start the backend
 *
 * @return bool
 */
bool server_start();

/**
 * @brief Run the wayland event loop
 */
void server_run();

void server_fini();

#endif //SYCAMORE_SERVER_H