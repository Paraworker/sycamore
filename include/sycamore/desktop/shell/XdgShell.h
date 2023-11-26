#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include "sycamore/defines.h"
#include "sycamore/desktop/XdgToplevel.h"
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
            return {};
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
        m_newToplevel
        .connect(handle->events.new_toplevel)
        .set([](void* data)
        {
            XdgToplevel::create(static_cast<wlr_xdg_toplevel*>(data));
        });

        m_destroy
        .connect(handle->events.destroy)
        .set([this](void*)
        {
            delete this;
        });
    }

    ~XdgShell() = default;

private:
    wlr_xdg_shell* m_handle;

private:
    Listener m_newToplevel;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_XDG_SHELL_H