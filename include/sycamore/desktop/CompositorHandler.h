#ifndef SYCAMORE_COMPOSITOR_HANDLER_H
#define SYCAMORE_COMPOSITOR_HANDLER_H

#include "sycamore/input/Seat.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

namespace sycamore
{

struct SurfaceHandler
{
    Listener destroy;

    explicit SurfaceHandler(wlr_surface* handle)
    {
        destroy = [this](auto)
        {
            delete this;
        };
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

    explicit CompositorHandler(wlr_compositor* handle)
    {
        newSurface = [](void* data)
        {
            new SurfaceHandler{static_cast<wlr_surface*>(data)};
        };
        newSurface.connect(handle->events.new_surface);

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(handle->events.destroy);
    }

    ~CompositorHandler() = default;
};

}

#endif //SYCAMORE_COMPOSITOR_HANDLER_H