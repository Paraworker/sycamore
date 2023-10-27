#include "sycamore/desktop/Surface.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/output/Output.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Core Core::instance{};

bool Core::init() {
    spdlog::info("Init Core");

    wlr_log_init(WLR_DEBUG, nullptr);

    if (display = wl_display_create(); !display) {
        spdlog::error("Create wl_display failed");
        return false;
    }

    eventLoop = wl_display_get_event_loop(display);

    if (backend = wlr_backend_autocreate(display, &session); !backend) {
        spdlog::error("Create wlr_backend failed");
        return false;
    }

    if (renderer = wlr_renderer_autocreate(backend); !renderer) {
        spdlog::error("Create wlr_renderer failed");
        return false;
    }

    wlr_renderer_init_wl_shm(renderer, display);

    if (wlr_renderer_get_dmabuf_texture_formats(renderer)) {
        wlr_drm_create(display, renderer);
        dmabuf = wlr_linux_dmabuf_v1_create_with_renderer(display, 4, renderer);
    }

    if (allocator = wlr_allocator_autocreate(backend, renderer); !allocator) {
        spdlog::error("Create wlr_allocator failed");
        return false;
    }

    if (compositor = wlr_compositor_create(display, WLR_COMPOSITOR_VERSION, renderer); !compositor) {
        spdlog::error("Create wlr_compositor failed");
        return false;
    }

    wlr_subcompositor_create(display);

    if (presentation = wlr_presentation_create(display, backend); !presentation) {
        spdlog::error("Create wlr_presentation failed");
        return false;
    }

    if (gestures = wlr_pointer_gestures_v1_create(display); !gestures) {
        spdlog::error("Create wlr_pointer_gestures_v1 failed");
        return false;
    }

    if (outputLayout = OutputLayout::create(); !outputLayout) {
        spdlog::error("Create OutputLayout failed");
        return false;
    }

    if (scene = Scene::create(outputLayout->getHandle(), presentation, dmabuf); !scene) {
        spdlog::error("Create Scene failed");
        return false;
    }

    if (seat = Seat::create(display, outputLayout->getHandle(), "seat0"); !seat) {
        spdlog::error("Create Seat failed");
        return false;
    }

    if (!XdgShell::create(display, XDG_SHELL_VERSION)) {
        spdlog::error("Create XdgShell failed");
        return false;
    }

    if (!LayerShell::create(display, LAYER_SHELL_VERSION)) {
        spdlog::error("Create LayerShell failed");
        return false;
    }

    if (m_socket = wl_display_add_socket_auto(display); m_socket.empty()) {
        spdlog::error("Open Wayland socket failed");
        return false;
    }

    wlr_xdg_output_manager_v1_create(display, outputLayout->getHandle());
    wlr_output_manager_v1_create(display);
    wlr_export_dmabuf_manager_v1_create(display);
    wlr_data_device_manager_create(display);
    wlr_data_control_manager_v1_create(display);
    wlr_primary_selection_v1_device_manager_create(display);
    wlr_screencopy_manager_v1_create(display);
    wlr_viewporter_create(display);
    wlr_gamma_control_manager_v1_create(display);
    wlr_single_pixel_buffer_manager_v1_create(display);

    m_newInput.set(&backend->events.new_input, [](void* data) {
        InputManager::instance.onNewDevice(static_cast<wlr_input_device*>(data));
    });

    m_newOutput.set(&backend->events.new_output, [](void* data) {
        Output::create(static_cast<wlr_output*>(data));
    });

    m_newSurface.set(&compositor->events.new_surface, [](void* data) {
        Surface::create(static_cast<wlr_surface*>(data));
    });

    setenv("WAYLAND_DISPLAY", m_socket.c_str(), true);
    setenv("XDG_CURRENT_DESKTOP", "Sycamore", true);

    return true;
}

void Core::uninit() {
    m_newInput.disconnect();
    m_newOutput.disconnect();
    m_newSurface.disconnect();

    wl_display_destroy_clients(display);
    wl_display_destroy(display);

    m_socket.clear();

    seat.reset();
    scene.reset();
    outputLayout.reset();
}

bool Core::start() const {
    spdlog::info("Starting Backend");

    if (!wlr_backend_start(backend)) {
        spdlog::error("Start Backend failed");
        return false;
    }

    return true;
}

void Core::run() const {
    spdlog::info("Running Compositor on WAYLAND_DISPLAY: {}", m_socket);
    wl_display_run(display);
}

NAMESPACE_SYCAMORE_END