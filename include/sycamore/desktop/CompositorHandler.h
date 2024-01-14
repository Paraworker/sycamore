#ifndef SYCAMORE_COMPOSITOR_HANDLER_H
#define SYCAMORE_COMPOSITOR_HANDLER_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

namespace sycamore
{

inline constexpr auto WLR_COMPOSITOR_VERSION = 6;

struct SurfaceHandler
{
    Listener destroy;

    static void create(wlr_surface* handle)
    {
        new SurfaceHandler{handle};
    }

    explicit SurfaceHandler(wlr_surface* handle)
    {
        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~SurfaceHandler()
    {
        core.seat->input->rebasePointer();
    }
};

struct CompositorHandler
{
    Listener newSurface;
    Listener destroy;

    static void create(wl_display* display, wlr_renderer* renderer)
    {
        new CompositorHandler{display, renderer};
    }

    explicit CompositorHandler(wl_display* display, wlr_renderer* renderer)
    {
        auto handle = wlr_compositor_create(display, WLR_COMPOSITOR_VERSION, renderer);
        if (!handle)
        {
            throw std::runtime_error("Create wlr_compositor failed!");
        }

        newSurface.notify([](void* data)
        {
            SurfaceHandler::create(static_cast<wlr_surface*>(data));
        });
        newSurface.connect(handle->events.new_surface);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~CompositorHandler() = default;
};

}

#endif //SYCAMORE_COMPOSITOR_HANDLER_H