#include <stdlib.h>
#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_single_pixel_buffer_v1.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/layer_shell.h"
#include "sycamore/desktop/shell/xdg_shell.h"
#include "sycamore/input/keybinding.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"

struct sycamore_server server = {0};

bool server_init() {
    wlr_log(WLR_INFO, "Initializing Wayland server");

    wl_list_init(&server.all_outputs);
    wl_list_init(&server.mapped_views);
    server.focused_view.view = NULL;

    server.wl_display = wl_display_create();
    server.backend = wlr_backend_autocreate(server.wl_display);
    if (!server.backend) {
        wlr_log(WLR_ERROR, "Unable to create backend");
        return false;
    }

    server.backend_new_input.notify = handle_backend_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.backend_new_input);
    server.backend_new_output.notify = handle_backend_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.backend_new_output);

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        wlr_log(WLR_ERROR, "Unable to create renderer");
        return false;
    }

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        wlr_log(WLR_ERROR, "Unable to create allocator");
        return false;
    }

    server.compositor = wlr_compositor_create(server.wl_display, server.renderer);
    if (!server.compositor) {
        wlr_log(WLR_ERROR, "Unable to create compositor");
        return false;
    }

    wlr_subcompositor_create(server.wl_display);

    server.output_layout = wlr_output_layout_create();
    if (!server.output_layout) {
        wlr_log(WLR_ERROR, "Unable to create output_layout");
        return false;
    }

    server.presentation = wlr_presentation_create(server.wl_display, server.backend);
    if (!server.presentation) {
        wlr_log(WLR_ERROR, "Unable to create presentation");
        return false;
    }

    server.seat = sycamore_seat_create(server.wl_display, server.output_layout);
    if (!server.seat) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_seat");
        return false;
    }

    server.scene = sycamore_scene_create(server.output_layout, server.presentation);
    if (!server.scene) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_scene");
        return false;
    }

    server.xdg_shell = sycamore_xdg_shell_create(server.wl_display);
    if (!server.xdg_shell) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_xdg_shell");
        return false;
    }

    server.layer_shell = sycamore_layer_shell_create(server.wl_display);
    if (!server.layer_shell) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_layer_shell");
        return false;
    }

    server.keybinding_manager = sycamore_keybinding_manager_create();
    if (!server.keybinding_manager) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_keybinding_manager");
        return false;
    }

    wlr_export_dmabuf_manager_v1_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);
    wlr_data_control_manager_v1_create(server.wl_display);
    wlr_primary_selection_v1_device_manager_create(server.wl_display);
    wlr_screencopy_manager_v1_create(server.wl_display);
    wlr_viewporter_create(server.wl_display);
    wlr_gamma_control_manager_v1_create(server.wl_display);
    wlr_xdg_output_manager_v1_create(server.wl_display, server.output_layout);
    wlr_single_pixel_buffer_manager_v1_create(server.wl_display);

    server.socket = wl_display_add_socket_auto(server.wl_display);
    if (!server.socket) {
        wlr_log(WLR_ERROR, "Unable to open wayland socket");
        return false;
    }

    return true;
}

void server_fini() {
    if (server.backend) {
        wl_list_remove(&server.backend_new_input.link);
        wl_list_remove(&server.backend_new_output.link);
        wlr_backend_destroy(server.backend);
        wl_display_destroy_clients(server.wl_display);
        wl_display_destroy(server.wl_display);
    }

    if (server.output_layout) {
        wlr_output_layout_destroy(server.output_layout);
    }

    if (server.seat) {
        sycamore_seat_destroy(server.seat);
    }

    if (server.scene) {
        sycamore_scene_destroy(server.scene);
    }

    if (server.xdg_shell) {
        sycamore_xdg_shell_destroy(server.xdg_shell);
    }

    if (server.layer_shell) {
        sycamore_layer_shell_destroy(server.layer_shell);
    }

    if (server.keybinding_manager) {
        sycamore_keybinding_manager_destroy(server.keybinding_manager);
    }
}

/* Start the backend */
bool server_start() {
    wlr_log(WLR_INFO, "Starting backend on wayland display '%s'",
            server.socket);

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Unable to start backend");
        return false;
    }

    return true;
}

/* Run the wayland event loop */
void server_run() {
    wlr_log(WLR_INFO, "Running Sycamore on WAYLAND_DISPLAY=%s",
            server.socket);

    wl_display_run(server.wl_display);
}