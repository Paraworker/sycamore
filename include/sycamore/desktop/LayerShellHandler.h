#ifndef SYCAMORE_LAYER_SHELL_HANDLER_H
#define SYCAMORE_LAYER_SHELL_HANDLER_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <stdexcept>

namespace sycamore
{

inline constexpr auto LAYER_SHELL_VERSION = 4;

struct LayerShellHandler
{
    Listener newSurface;
    Listener destroy;

    static void create(wl_display* display)
    {
        new LayerShellHandler{display};
    }

    explicit LayerShellHandler(wl_display* display)
    {
        auto handle = wlr_layer_shell_v1_create(display, LAYER_SHELL_VERSION);
        if (!handle)
        {
            throw std::runtime_error("Create wlr_layer_shell_v1 failed!");
        }

        newSurface.notify([](void* data)
        {
            Layer::create(static_cast<wlr_layer_surface_v1*>(data));
        });
        newSurface.connect(handle->events.new_surface);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~LayerShellHandler() = default;
};

}

#endif //SYCAMORE_LAYER_SHELL_HANDLER_H