#ifndef SYCAMORE_XDG_SHELL_HANDLER_H
#define SYCAMORE_XDG_SHELL_HANDLER_H

#include "sycamore/desktop/XdgToplevel.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

struct XdgShellHandler
{
    Listener newToplevel;
    Listener destroy;

    explicit XdgShellHandler(wlr_xdg_shell* handle)
    {
        newToplevel = [](void* data)
        {
            new XdgToplevel{static_cast<wlr_xdg_toplevel*>(data)};
        };
        newToplevel.connect(handle->events.new_toplevel);

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(handle->events.destroy);
    }
};

}

#endif //SYCAMORE_XDG_SHELL_HANDLER_H