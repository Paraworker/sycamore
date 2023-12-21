#ifndef SYCAMORE_LAYER_SHELL_H
#define SYCAMORE_LAYER_SHELL_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <stdexcept>

#define LAYER_SHELL_VERSION 4

namespace sycamore
{

class LayerShell
{
public:
    /**
     * @brief Init wlr layer shell
     */
    static void init(wl_display* display)
    {
        auto handle = wlr_layer_shell_v1_create(display, LAYER_SHELL_VERSION);
        if (!handle)
        {
            throw std::runtime_error("Create wlr_layer_shell_v1 failed");
        }

        // Be destroyed by listener
        new LayerShell{handle};
    }

    LayerShell(const LayerShell&) = delete;
    LayerShell(LayerShell&&) = delete;
    LayerShell& operator=(const LayerShell&) = delete;
    LayerShell& operator=(LayerShell&&) = delete;

private:
    explicit LayerShell(wlr_layer_shell_v1* handle) noexcept
    {
        m_newSurface
        .connect(handle->events.new_surface)
        .set([](void* data)
        {
            Layer::create(static_cast<wlr_layer_surface_v1*>(data));
        });

        m_destroy
        .connect(handle->events.destroy)
        .set([this](auto)
        {
            delete this;
        });
    }

    ~LayerShell() = default;

private:
    Listener m_newSurface;
    Listener m_destroy;
};

}

#endif //SYCAMORE_LAYER_SHELL_H