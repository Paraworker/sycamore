#ifndef SYCAMORE_LAYER_SHELL_HANDLER_H
#define SYCAMORE_LAYER_SHELL_HANDLER_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

struct LayerShellHandler
{
    Listener newSurface;
    Listener destroy;

    explicit LayerShellHandler(wlr_layer_shell_v1* handle)
    {
        newSurface = [](void* data)
        {
            auto handle = static_cast<wlr_layer_surface_v1*>(data);

            // Confirm output
            if (!handle->output)
            {
                auto output = core.cursor.atOutput();
                if (!output)
                {
                    spdlog::error("No output under cursor for layerSurface");
                    wlr_layer_surface_v1_destroy(handle);
                    return;
                }

                handle->output = output->getHandle();
            }

            new Layer{handle};
        };
        newSurface.connect(handle->events.new_surface);

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(handle->events.destroy);
    }
};

}

#endif //SYCAMORE_LAYER_SHELL_HANDLER_H