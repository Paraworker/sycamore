#include "sycamore/Core.h"

#include "sycamore/desktop/shell/LayerShell.h"
#include "sycamore/desktop/shell/XdgShell.h"
#include "sycamore/desktop/Surface.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/output/Output.h"
#include "sycamore/output/OutputLayout.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

constexpr auto WLR_COMPOSITOR_VERSION = 6;
constexpr auto DEFAULT_SEAT           = "seat0";

Core Core::instance{};

// wlr_backend helper
struct Backend
{
    static wlr_backend* autocreate(wl_display* display, wlr_session*& session)
    {
        auto handle = wlr_backend_autocreate(display, &session);
        if (!handle)
        {
            return {};
        }

        // Be destroyed by listener
        new Backend{handle};

        return handle;
    }

    explicit Backend(wlr_backend* handle)
    {
        newInput.notify([](void* data)
        {
            InputManager::instance.onNewDevice(static_cast<wlr_input_device*>(data));
        });
        newInput.connect(handle->events.new_input);

        newOutput.notify([](void* data)
        {
            Output::create(static_cast<wlr_output*>(data));
        });
        newOutput.connect(handle->events.new_output);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~Backend() = default;

    Listener newInput;
    Listener newOutput;
    Listener destroy;
};

// wlr_compositor helper
struct Compositor
{
    static wlr_compositor* create(wl_display* display, wlr_renderer* renderer)
    {
        auto handle = wlr_compositor_create(display, WLR_COMPOSITOR_VERSION, renderer);
        if (!handle)
        {
            return {};
        }

        // Be destroyed by listener
        new Compositor{handle};

        return handle;
    }

    explicit Compositor(wlr_compositor* handle)
    {
        newSurface.notify([](void* data)
        {
            Surface::create(static_cast<wlr_surface*>(data));
        });
        newSurface.connect(handle->events.new_surface);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~Compositor() = default;

    Listener newSurface;
    Listener destroy;
};

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

    if (backend = Backend::autocreate(display, session); !backend)
    {
        spdlog::error("Create wlr_backend failed");
        return false;
    }

    if (renderer = wlr_renderer_autocreate(backend); !renderer)
    {
        spdlog::error("Create wlr_renderer failed");
        return false;
    }

    wlr_renderer_init_wl_shm(renderer, display);

    if (wlr_renderer_get_dmabuf_texture_formats(renderer))
    {
        linuxDmabuf = wlr_linux_dmabuf_v1_create_with_renderer(display, 4, renderer);
    }

    if (allocator = wlr_allocator_autocreate(backend, renderer); !allocator)
    {
        spdlog::error("Create wlr_allocator failed");
        return false;
    }

    if (compositor = Compositor::create(display, renderer); !compositor)
    {
        spdlog::error("Create wlr_compositor failed");
        return false;
    }

    wlr_subcompositor_create(display);

    wlr_presentation_create(display, backend);

    if (gestures = wlr_pointer_gestures_v1_create(display); !gestures)
    {
        spdlog::error("Create wlr_pointer_gestures_v1 failed");
        return false;
    }

    outputLayout = OutputLayout::create(display);
    seat         = Seat::create(display, DEFAULT_SEAT, outputLayout->getHandle());
    scene        = std::make_unique<Scene>(outputLayout->getHandle(), linuxDmabuf);

    XdgShell::create(display);
    LayerShell::create(display);

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

    if (!wlr_backend_start(backend))
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