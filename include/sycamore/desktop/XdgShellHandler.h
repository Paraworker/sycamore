#ifndef SYCAMORE_XDG_SHELL_HANDLER_H
#define SYCAMORE_XDG_SHELL_HANDLER_H

#include "sycamore/desktop/XdgToplevel.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <stdexcept>

namespace sycamore
{

inline constexpr auto XDG_SHELL_VERSION = 6;

struct XdgShellHandler
{
    Listener newToplevel;
    Listener destroy;

    static void create(wl_display* display)
    {
        new XdgShellHandler{display};
    }

    explicit XdgShellHandler(wl_display* display)
    {
        auto handle = wlr_xdg_shell_create(display, XDG_SHELL_VERSION);
        if (!handle)
        {
            throw std::runtime_error("Create wlr_xdg_shell failed!");
        }

        newToplevel.notify([](void* data)
        {
            XdgToplevel::create(static_cast<wlr_xdg_toplevel*>(data));
        });
        newToplevel.connect(handle->events.new_toplevel);

        destroy.notify([this](auto)
        {
            delete this;
        });
        destroy.connect(handle->events.destroy);
    }

    ~XdgShellHandler() = default;
};

}

#endif //SYCAMORE_XDG_SHELL_HANDLER_H