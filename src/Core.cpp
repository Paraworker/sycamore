#include "sycamore/desktop/shell/LayerShell.h"
#include "sycamore/desktop/shell/XdgShell.h"
#include "sycamore/desktop/Surface.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/output/Output.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

#define WLR_COMPOSITOR_VERSION  6
#define LAYER_SHELL_VERSION     4
#define XDG_SHELL_VERSION       3
#define DEFAULT_SEAT            "seat0"

namespace sycamore
{

Core Core::instance{};

static Core::Backend* backendAutoCreate(wl_display* display)
{
    auto backend = new Core::Backend{};

    if (backend->handle = wlr_backend_autocreate(display, &backend->session); !backend->handle)
    {
        spdlog::error("Create wlr_backend failed");
        delete backend;
        return {};
    }

    backend->newInput
    .connect(backend->handle->events.new_input)
    .set([](void* data)
    {
        InputManager::instance.onNewDevice(static_cast<wlr_input_device*>(data));
    });

    backend->newOutput
    .connect(backend->handle->events.new_output)
    .set([](void* data)
    {
        Output::create(static_cast<wlr_output*>(data));
    });

    backend->destroy
    .connect(backend->handle->events.destroy)
    .set([=](void*)
    {
        delete backend;
    });

    return backend;
}

static Core::Compositor* compositorCreate(wl_display* display, uint32_t version, wlr_renderer* renderer)
{
    auto compositor = new Core::Compositor{};

    if (compositor->handle = wlr_compositor_create(display, version, renderer); !compositor->handle)
    {
        spdlog::error("Create wlr_compositor failed");
        delete compositor;
        return {};
    }

    compositor->newSurface
    .connect(compositor->handle->events.new_surface)
    .set([](void* data)
    {
        Surface::create(static_cast<wlr_surface*>(data));
    });

    compositor->destroy
    .connect(compositor->handle->events.destroy)
    .set([=](void*)
    {
        delete compositor;
    });

    return compositor;
}

bool Core::setup()
{
    spdlog::info("Setup Server");

    wlr_log_init(WLR_DEBUG, nullptr);

    if (display = wl_display_create(); !display)
    {
        spdlog::error("Create wl_display failed");
        return false;
    }

    eventLoop = wl_display_get_event_loop(display);

    if (backend = backendAutoCreate(display); !backend)
    {
        spdlog::error("Create Backend failed");
        return false;
    }

    if (renderer = wlr_renderer_autocreate(backend->handle); !renderer)
    {
        spdlog::error("Create wlr_renderer failed");
        return false;
    }

    wlr_renderer_init_wl_shm(renderer, display);

    if (wlr_renderer_get_dmabuf_texture_formats(renderer))
    {
        wlr_drm_create(display, renderer);
        linuxDmabuf = wlr_linux_dmabuf_v1_create_with_renderer(display, 4, renderer);
    }

    if (allocator = wlr_allocator_autocreate(backend->handle, renderer); !allocator)
    {
        spdlog::error("Create wlr_allocator failed");
        return false;
    }

    if (compositor = compositorCreate(display, WLR_COMPOSITOR_VERSION, renderer); !compositor)
    {
        spdlog::error("Create Compositor failed");
        return false;
    }

    wlr_subcompositor_create(display);

    if (presentation = wlr_presentation_create(display, backend->handle); !presentation)
    {
        spdlog::error("Create wlr_presentation failed");
        return false;
    }

    if (gestures = wlr_pointer_gestures_v1_create(display); !gestures)
    {
        spdlog::error("Create wlr_pointer_gestures_v1 failed");
        return false;
    }

    if (outputLayout = OutputLayout::create(display); !outputLayout)
    {
        spdlog::error("Create OutputLayout failed");
        return false;
    }

    if (seat = Seat::create(display, outputLayout->getHandle(), DEFAULT_SEAT); !seat)
    {
        spdlog::error("Create Seat failed");
        return false;
    }

    if (scene = Scene::create(outputLayout->getHandle(), presentation, linuxDmabuf); !scene)
    {
        spdlog::error("Create Scene failed");
        return false;
    }

    if (!XdgShell::create(display, XDG_SHELL_VERSION))
    {
        spdlog::error("Create XdgShell failed");
        return false;
    }

    if (!LayerShell::create(display, LAYER_SHELL_VERSION))
    {
        spdlog::error("Create LayerShell failed");
        return false;
    }

    if (socket = wl_display_add_socket_auto(display); socket.empty())
    {
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

    setenv("WAYLAND_DISPLAY", socket.c_str(), true);
    setenv("XDG_CURRENT_DESKTOP", "Sycamore", true);

    return true;
}

void Core::teardown()
{
    wl_display_destroy_clients(display);
    wl_display_destroy(display);
}

bool Core::start() const
{
    spdlog::info("Starting Backend");

    if (!wlr_backend_start(backend->handle))
    {
        spdlog::error("Start Backend failed");
        return false;
    }

    return true;
}

void Core::run() const
{
    spdlog::info("Running Compositor on WAYLAND_DISPLAY: {}", socket);
    wl_display_run(display);
}

}