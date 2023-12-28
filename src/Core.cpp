#include "sycamore/desktop/shell/LayerShell.h"
#include "sycamore/desktop/shell/XdgShell.h"
#include "sycamore/desktop/Surface.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/output/Output.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

#define WLR_COMPOSITOR_VERSION  6
#define DEFAULT_SEAT            "seat0"

namespace sycamore
{

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

    explicit Backend(wlr_backend* handle) : handle{handle}
    {
        newInput
        .connect(handle->events.new_input)
        .set([](void* data)
        {
            InputManager::instance.onNewDevice(static_cast<wlr_input_device*>(data));
        });

        newOutput
        .connect(handle->events.new_output)
        .set([](void* data)
        {
            Output::create(static_cast<wlr_output*>(data));
        });

        destroy
        .connect(handle->events.destroy)
        .set([this](auto)
        {
            delete this;
        });
    }

    ~Backend() = default;

    wlr_backend* handle;

    Listener     newInput;
    Listener     newOutput;
    Listener     destroy;
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

    explicit Compositor(wlr_compositor* handle) : handle{handle}
    {
        newSurface
        .connect(handle->events.new_surface)
        .set([](void* data)
        {
            Surface::create(static_cast<wlr_surface*>(data));
        });

        destroy
        .connect(handle->events.destroy)
        .set([this](auto)
        {
            delete this;
        });
    }

    ~Compositor() = default;

    wlr_compositor* handle;

    Listener        newSurface;
    Listener        destroy;
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
        wlr_drm_create(display, renderer);
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

    if (outputLayout = OutputLayout::create(display); !outputLayout)
    {
        spdlog::error("Create OutputLayout failed");
        return false;
    }

    seat = Seat::create(display, DEFAULT_SEAT, outputLayout->getHandle());

    if (scene = Scene::create(outputLayout->getHandle(), linuxDmabuf); !scene)
    {
        spdlog::error("Create Scene failed");
        return false;
    }

    XdgShell::init(display);
    LayerShell::init(display);

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