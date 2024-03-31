#include "sycamore/Core.h"

#include "sycamore/desktop/CompositorHandler.h"
#include "sycamore/desktop/LayerShellHandler.h"
#include "sycamore/desktop/XdgShellHandler.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputManager.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

static constexpr auto WLR_COMPOSITOR_VERSION = 6;
static constexpr auto XDG_SHELL_VERSION      = 6;
static constexpr auto LAYER_SHELL_VERSION    = 4;

struct BackendHandler
{
    Listener newInput;
    Listener newOutput;
    Listener destroy;

    explicit BackendHandler(wlr_backend* handle)
    {
        newInput = [](void* data)
        {
            inputManager.addDevice(static_cast<wlr_input_device*>(data));
        };
        newInput.connect(handle->events.new_input);

        newOutput = [](void* data)
        {
            outputManager.addOutput(static_cast<wlr_output*>(data));
        };
        newOutput.connect(handle->events.new_output);

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(handle->events.destroy);
    }
};

Core::Core()
{
    wlr_log_init(WLR_DEBUG, nullptr);

    display   = wl_display_create();
    eventLoop = wl_display_get_event_loop(display);

    if (backend = wlr_backend_autocreate(eventLoop, &session); !backend)
    {
        throw std::runtime_error{"Autocreate wlr_backend failed!"};
    }

    new BackendHandler{backend};

    if (renderer = wlr_renderer_autocreate(backend); !renderer)
    {
        throw std::runtime_error{"Autocreate wlr_renderer failed!"};
    }

    wlr_renderer_init_wl_shm(renderer, display);

    if (wlr_renderer_get_dmabuf_texture_formats(renderer))
    {
        linuxDmabuf = wlr_linux_dmabuf_v1_create_with_renderer(display, 4, renderer);
    }

    if (allocator = wlr_allocator_autocreate(backend, renderer); !allocator)
    {
        throw std::runtime_error{"Autocreate wlr_allocator failed!"};
    }

    compositor = wlr_compositor_create(display, WLR_COMPOSITOR_VERSION, renderer);
    new CompositorHandler{compositor};

    wlr_subcompositor_create(display);

    outputLayout = wlr_output_layout_create(display);

    scene.init(outputLayout, linuxDmabuf);

    cursor.init(outputLayout);

    seat = std::make_unique<Seat>(display, "seat0");

    pointerGestures = wlr_pointer_gestures_v1_create(display);

    xdgShell = wlr_xdg_shell_create(display, XDG_SHELL_VERSION);
    new XdgShellHandler{xdgShell};

    layerShell = wlr_layer_shell_v1_create(display, LAYER_SHELL_VERSION);
    new LayerShellHandler{layerShell};

    wlr_presentation_create(display, backend);
    wlr_xdg_output_manager_v1_create(display, outputLayout);
    wlr_output_manager_v1_create(display);
    wlr_data_device_manager_create(display);
    wlr_data_control_manager_v1_create(display);
    wlr_export_dmabuf_manager_v1_create(display);
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
        throw std::runtime_error{"Open Wayland socket failed!"};
    }

    setenv("WAYLAND_DISPLAY", socket.c_str(), true);

    spdlog::info("Using WAYLAND_DISPLAY={}", socket);

    // Start backend
    if (!wlr_backend_start(backend))
    {
        throw std::runtime_error{"Start Backend failed!"};
    }
}

void Core::run() const
{
    wl_display_run(display);
}

void Core::terminate() const
{
    wl_display_terminate(display);
}

void Core::switchVt(uint32_t vt) const
{
    if (session)
    {
        wlr_session_change_vt(session, vt);
    }
}

}