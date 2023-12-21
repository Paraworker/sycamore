#ifndef SYCAMORE_XDG_SHELL_H
#define SYCAMORE_XDG_SHELL_H

#include "sycamore/desktop/XdgToplevel.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <stdexcept>

#define XDG_SHELL_VERSION 6

namespace sycamore
{

class XdgShell
{
public:
    /**
     * @brief Init XDG shell
     */
    static void init(wl_display* display)
    {
        auto handle = wlr_xdg_shell_create(display, XDG_SHELL_VERSION);
        if (!handle)
        {
            throw std::runtime_error("Create wlr_xdg_shell failed");
        }

        // Be destroyed by listener
        new XdgShell{handle};
    }

    XdgShell(const XdgShell&) = delete;
    XdgShell(XdgShell&&) = delete;
    XdgShell& operator=(const XdgShell&) = delete;
    XdgShell& operator=(XdgShell&&) = delete;

private:
    explicit XdgShell(wlr_xdg_shell* handle) noexcept
    {
        m_newToplevel
        .connect(handle->events.new_toplevel)
        .set([](void* data)
        {
            XdgToplevel::create(static_cast<wlr_xdg_toplevel*>(data));
        });

        m_destroy
        .connect(handle->events.destroy)
        .set([this](auto)
        {
            delete this;
        });
    }

    ~XdgShell() = default;

private:
    Listener m_newToplevel;
    Listener m_destroy;
};

}

#endif //SYCAMORE_XDG_SHELL_H