#include "sycamore/Core.h"

#include "sycamore/desktop/CompositorHandler.h"
#include "sycamore/desktop/LayerShellHandler.h"
#include "sycamore/desktop/XdgShellHandler.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/output/OutputManager.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

static constexpr auto DEFAULT_SEAT = "seat0";

struct BackendHandler
{
    Listener newInput;
    Listener newOutput;
    Listener destroy;

    static void create(wlr_backend* handle)
    {
        new BackendHandler{handle};
    }

    explicit BackendHandler(wlr_backend* handle)
    {
        newInput.notify([](void* data)
        {
            inputManager.newDevice(static_cast<wlr_input_device*>(data));
        });
        newInput.connect(handle->events.new_input);

        newOutput.notify([](void* data)
        {
            outputManager.newOutput(static_cast<wlr_output*>(data));
        });
        newOutput.connect(handle->events.new_output);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~BackendHandler() = default;
};

Core::Core()
{
    wlr_log_init(WLR_DEBUG, nullptr);

    if (display = wl_display_create(); !display)
    {
        throw std::runtime_error("Create wl_display failed!");
    }

    eventLoop = wl_display_get_event_loop(display);

    if (backend = wlr_backend_autocreate(eventLoop, &session); !backend)
    {
        throw std::runtime_error("Autocreate wlr_backend failed!");
    }

    BackendHandler::create(backend);

    if (renderer = wlr_renderer_autocreate(backend); !renderer)
    {
        throw std::runtime_error("Autocreate wlr_renderer failed!");
    }

    wlr_renderer_init_wl_shm(renderer, display);

    if (wlr_renderer_get_dmabuf_texture_formats(renderer))
    {
        linuxDmabuf = wlr_linux_dmabuf_v1_create_with_renderer(display, 4, renderer);
    }

    if (allocator = wlr_allocator_autocreate(backend, renderer); !allocator)
    {
        throw std::runtime_error("Autocreate wlr_allocator failed!");
    }

    CompositorHandler::create(display, renderer);

    wlr_subcompositor_create(display);

    wlr_presentation_create(display, backend);

    if (outputLayout = wlr_output_layout_create(display); !outputLayout)
    {
        throw std::runtime_error("Create wlr_output_layout failed!");
    }

    seat = Seat::create(display, DEFAULT_SEAT);
    seat->cursor.attachOutputLayout(outputLayout);

    scene.init(outputLayout, linuxDmabuf);

    XdgShellHandler::create(display);
    LayerShellHandler::create(display);

    if (gestures = wlr_pointer_gestures_v1_create(display); !gestures)
    {
        throw std::runtime_error("Create wlr_pointer_gestures_v1 failed!");
    }

    wlr_xdg_output_manager_v1_create(display, outputLayout);
    wlr_output_manager_v1_create(display);
    wlr_export_dmabuf_manager_v1_create(display);
    wlr_data_device_manager_create(display);
    wlr_data_control_manager_v1_create(display);
    wlr_primary_selection_v1_device_manager_create(display);
    wlr_screencopy_manager_v1_create(display);
    wlr_viewporter_create(display);
    wlr_gamma_control_manager_v1_create(display);
    wlr_single_pixel_buffer_manager_v1_create(display);
}

Core::~Core()
{
    wl_display_destroy_clients(display);
    wl_display_destroy(display);
}

void Core::start()
{
    // Add socket
    if (socket = wl_display_add_socket_auto(display); socket.empty())
    {
        throw std::runtime_error("Open Wayland socket failed!");
    }

    setenv("WAYLAND_DISPLAY", socket.c_str(), true);

    spdlog::info("Using WAYLAND_DISPLAY={}", socket);

    // Start backend
    if (!wlr_backend_start(backend))
    {
        throw std::runtime_error("Start Backend failed!");
    }
}

void Core::run() const
{
    wl_display_run(display);
}

}