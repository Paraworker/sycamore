#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include "sycamore/desktop/XdgToplevel.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <spdlog/spdlog.h>

#define XDG_SHELL_VERSION 3

namespace sycamore
{

class XdgShell
{
public:
    /**
     * @brief Init XDG shell
     */
    static bool init(wl_display* display)
    {
        auto handle = wlr_xdg_shell_create(display, XDG_SHELL_VERSION);
        if (!handle)
        {
            spdlog::error("Create wlr_xdg_shell failed");
            return false;
        }

        new XdgShell{handle};

        return true;
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

}

#endif //SYCAMORE_XDG_SHELL_H