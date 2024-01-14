#ifndef SYCAMORE_KEYBINDING_DISPATCHERS_H
#define SYCAMORE_KEYBINDING_DISPATCHERS_H

#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/utils/process.h"
#include "sycamore/Core.h"

#include <string>

namespace sycamore
{

struct Spawn
{
    void operator()() const
    {
        spawn(command.c_str());
    }

    std::string command;
};

struct CloseFocusedToplevel
{
    void operator()() const
    {
        if (auto toplevel = ShellManager::instance.getFocusState().toplevel; toplevel)
        {
            toplevel->close();
        }
    }
};

struct CycleToplevel
{
    void operator()() const
    {
        ShellManager::instance.cycleToplevel();
    }
};

struct Terminate
{
    void operator()() const
    {
        wl_display_terminate(core.display);
    }
};

template<unsigned int vt>
requires (vt >= 1 && vt <= 6)
struct SwitchVT
{
    void operator()() const
    {
        if (auto session = core.session; session)
        {
            wlr_session_change_vt(session, vt);
        }
    }
};

}

#endif //SYCAMORE_KEYBINDING_DISPATCHERS_H