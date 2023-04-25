#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_drm.h>
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
#include "sycamore/defines.h"
#include "sycamore/desktop/shell/layer_shell/layer_shell.h"
#include "sycamore/desktop/shell/xdg_shell/xdg_shell.h"
#include "sycamore/input/keybinding.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/scene.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"

Server server = {0};

bool serverInit() {
    wlr_log(WLR_INFO, "Initializing Wayland server");

    wl_list_init(&server.allOutputs);
    wl_list_init(&server.mappedViews);
    server.focusedView.view = NULL;
    server.focusedLayer     = NULL;

    server.wlDisplay = wl_display_create();
    server.backend = wlr_backend_autocreate(server.wlDisplay, &server.session);
    if (!server.backend) {
        wlr_log(WLR_ERROR, "Unable to create backend");
        return false;
    }

    server.backendNewInput.notify = onBackendNewInput;
    wl_signal_add(&server.backend->events.new_input, &server.backendNewInput);
    server.backendNewOutput.notify = onBackendNewOutput;
    wl_signal_add(&server.backend->events.new_output, &server.backendNewOutput);

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        wlr_log(WLR_ERROR, "Unable to create renderer");
        return false;
    }

    wlr_renderer_init_wl_shm(server.renderer, server.wlDisplay);

    if (wlr_renderer_get_dmabuf_texture_formats(server.renderer) != NULL) {
        wlr_drm_create(server.wlDisplay, server.renderer);
        server.dmabuf = wlr_linux_dmabuf_v1_create_with_renderer(
                server.wlDisplay, 4, server.renderer);
    }

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        wlr_log(WLR_ERROR, "Unable to create allocator");
        return false;
    }

    server.compositor = wlr_compositor_create(server.wlDisplay, COMPOSITOR_VERSION, server.renderer);
    if (!server.compositor) {
        wlr_log(WLR_ERROR, "Unable to create compositor");
        return false;
    }

    wlr_subcompositor_create(server.wlDisplay);

    server.outputLayout = wlr_output_layout_create();
    if (!server.outputLayout) {
        wlr_log(WLR_ERROR, "Unable to create outputLayout");
        return false;
    }

    server.presentation = wlr_presentation_create(server.wlDisplay, server.backend);
    if (!server.presentation) {
        wlr_log(WLR_ERROR, "Unable to create presentation");
        return false;
    }

    server.scene = sceneCreate(server.outputLayout, server.presentation, server.dmabuf);
    if (!server.scene) {
        wlr_log(WLR_ERROR, "Unable to create Scene");
        return false;
    }

    server.seat = seatCreate(server.wlDisplay, server.outputLayout);
    if (!server.seat) {
        wlr_log(WLR_ERROR, "Unable to create Seat");
        return false;
    }

    server.xdgShell = xdgShellCreate(server.wlDisplay);
    if (!server.xdgShell) {
        wlr_log(WLR_ERROR, "Unable to create XdgShell");
        return false;
    }

    server.layerShell = layerShellCreate(server.wlDisplay);
    if (!server.layerShell) {
        wlr_log(WLR_ERROR, "Unable to create LayerShell");
        return false;
    }

    server.keybindingManager = keybindingManagerCreate();
    if (!server.keybindingManager) {
        wlr_log(WLR_ERROR, "Unable to create KeybindingManager");
        return false;
    }

    wlr_export_dmabuf_manager_v1_create(server.wlDisplay);
    wlr_data_device_manager_create(server.wlDisplay);
    wlr_data_control_manager_v1_create(server.wlDisplay);
    wlr_primary_selection_v1_device_manager_create(server.wlDisplay);
    wlr_screencopy_manager_v1_create(server.wlDisplay);
    wlr_viewporter_create(server.wlDisplay);
    wlr_gamma_control_manager_v1_create(server.wlDisplay);
    wlr_xdg_output_manager_v1_create(server.wlDisplay, server.outputLayout);
    wlr_single_pixel_buffer_manager_v1_create(server.wlDisplay);

    server.socket = wl_display_add_socket_auto(server.wlDisplay);
    if (!server.socket) {
        wlr_log(WLR_ERROR, "Unable to open wayland socket");
        return false;
    }

    return true;
}

void serverUninit() {
    if (server.backend) {
        wl_list_remove(&server.backendNewInput.link);
        wl_list_remove(&server.backendNewOutput.link);
        wlr_backend_destroy(server.backend);
        wl_display_destroy_clients(server.wlDisplay);
        wl_display_destroy(server.wlDisplay);
    }

    if (server.outputLayout) {
        wlr_output_layout_destroy(server.outputLayout);
    }

    if (server.keybindingManager) {
        keybindingManagerDestroy(server.keybindingManager);
    }

    if (server.xdgShell) {
        xdgShellDestroy(server.xdgShell);
    }

    if (server.layerShell) {
        layerShellDestroy(server.layerShell);
    }

    if (server.seat) {
        seatDestroy(server.seat);
    }

    if (server.scene) {
        sceneDestroy(server.scene);
    }
}

bool serverStart() {
    wlr_log(WLR_INFO, "Starting backend on WAYLAND_DISPLAY=%s", server.socket);

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Unable to start backend");
        return false;
    }

    return true;
}

void serverRun() {
    wlr_log(WLR_INFO, "Running Sycamore on WAYLAND_DISPLAY=%s", server.socket);

    wl_display_run(server.wlDisplay);
}