#ifndef SYCAMORE_LAYER_SHELL_H
#define SYCAMORE_LAYER_SHELL_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <spdlog/spdlog.h>

#define LAYER_SHELL_VERSION 4

namespace sycamore
{

class LayerShell
{
public:
    /**
     * @brief Init wlr layer shell
     */
    static bool init(wl_display* display)
    {
        auto handle = wlr_layer_shell_v1_create(display, LAYER_SHELL_VERSION);
        if (!handle)
        {
            spdlog::error("Create wlr_layer_shell_v1 failed");
            return false;
        }

        new LayerShell{handle};

        return true;
    }

    LayerShell(const LayerShell&) = delete;
    LayerShell(LayerShell&&) = delete;
    LayerShell& operator=(const LayerShell&) = delete;
    LayerShell& operator=(LayerShell&&) = delete;

private:
    explicit LayerShell(wlr_layer_shell_v1* handle) : m_handle{handle}
    {
        m_newSurface
        .connect(handle->events.new_surface)
        .set([](void* data)
        {
            Layer::create(static_cast<wlr_layer_surface_v1*>(data));
        });

        m_destroy
        .connect(handle->events.destroy)
        .set([this](void*)
        {
            delete this;
        });
    }

    ~LayerShell() = default;

private:
    wlr_layer_shell_v1* m_handle;

private:
    Listener m_newSurface;
    Listener m_destroy;
};

}

#endif //SYCAMORE_LAYER_SHELL_H