#ifndef SYCAMORE_KEYBINDING_DISPATCHERS_H
#define SYCAMORE_KEYBINDING_DISPATCHERS_H

#include "sycamore/defines.h"
#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/KeybindingManager.h"
#include "sycamore/utils/process.h"
#include "sycamore/Core.h"

#include <string>

NAMESPACE_SYCAMORE_BEGIN

struct Spawn
{
    void operator()() const
    {
        spawn(cmd.c_str());
    }

    std::string cmd;
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
        wl_display_terminate(Core::instance.display);
    }
};

template<unsigned int vt>
requires (vt >= 1 && vt <= 6)
struct SwitchVT
{
    void operator()() const
    {
        wlr_session_change_vt(Core::instance.session, vt);
    }
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_KEYBINDING_DISPATCHERS_H