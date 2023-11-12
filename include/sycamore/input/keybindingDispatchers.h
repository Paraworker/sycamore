#ifndef SYCAMORE_KEYBINDING_DISPATCHERS_H
#define SYCAMORE_KEYBINDING_DISPATCHERS_H

#include "sycamore/defines.h"
#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/View.h"
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

struct CloseFocusedView
{
    void operator()() const
    {
        if (auto view = ShellManager::instance.getFocusState().view; view)
        {
            view->close();
        }
    }
};

struct CycleView
{
    void operator()() const
    {
        if (ShellManager::instance.getMappedViewList().size() < 2)
        {
            return;
        }

        View* next = wl_container_of(ShellManager::instance.getMappedViewList().getHandle().prev, next, link);
        ShellManager::instance.setFocus(next);
        Core::instance.seat->getInput().rebasePointer();
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