#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include "sycamore/defines.h"
#include "sycamore/desktop/XdgView.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

class XdgShell
{
public:
    /**
     * @brief Create XdgShell
     * @return nullptr on failure
     */
    static XdgShell* create(wl_display* display, uint32_t version)
    {
        auto handle = wlr_xdg_shell_create(display, version);
        if (!handle)
        {
            spdlog::error("Create wlr_xdg_shell failed");
            return nullptr;
        }

        return new XdgShell{handle};
    }

    XdgShell(const XdgShell&) = delete;
    XdgShell(XdgShell&&) = delete;
    XdgShell& operator=(const XdgShell&) = delete;
    XdgShell& operator=(XdgShell&&) = delete;

private:
    explicit XdgShell(wlr_xdg_shell* handle) : m_handle{handle}
    {
        m_newSurface
        .connect(handle->events.new_surface)
        .set([](void* data)
        {
            auto xdgSurface = static_cast<wlr_xdg_surface*>(data);

            if (xdgSurface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
            {
                XdgView::create(xdgSurface->toplevel);
            }
        });

        m_destroy
        .connect(handle->events.destroy)
        .set([this](void*) { delete this; });
    }

    ~XdgShell() = default;

private:
    wlr_xdg_shell* m_handle;

private:
    Listener m_newSurface;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_XDG_SHELL_H