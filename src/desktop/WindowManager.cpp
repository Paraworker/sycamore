#include "sycamore/desktop/WindowManager.h"

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

namespace sycamore
{

WindowManager::WindowManager()
    : m_focusState{}
    , m_fullscreenCount{0}
{}

WindowManager::~WindowManager() = default;

void WindowManager::focusToplevel(Toplevel& toplevel)
{
    // Note: this function only deals with keyboard focus.
    if (m_focusState.toplevel == &toplevel)
    {
        return;
    }

    if (m_focusState.toplevel)
    {
        // Deactivate prev toplevel
        m_focusState.toplevel->setActivated(false);
    }

    // Bring new toplevel to front
    toplevel.toFront();
    m_mappedToplevels.splice(m_mappedToplevels.end(), m_mappedToplevels, toplevel.iter);

    // Activate new toplevel
    toplevel.setActivated(true);

    if (!m_focusState.layer)
    {
        core.seat->setKeyboardFocus(toplevel.baseSurface());
    }

    m_focusState.toplevel = &toplevel;
}

void WindowManager::focusLayer(Layer& layer)
{
    if (m_focusState.layer == &layer)
    {
        return;
    }

    m_focusState.layer = &layer;

    core.seat->setKeyboardFocus(layer.baseSurface());
}

void WindowManager::mapToplevel(Toplevel& toplevel, bool maximized, bool fullscreen)
{
    // Layout stuff
    auto output = toplevel.output();

    if (output)
    {
        toplevel.setToOutputCenter(*output);

        if (maximized)
        {
            maximizeRequest(toplevel, *output);
        }

        if (fullscreen)
        {
            fullscreenRequest(toplevel, *output);
        }
    }

    toplevel.iter = m_mappedToplevels.emplace(m_mappedToplevels.end(), &toplevel);
    focusToplevel(toplevel);

    wl_signal_emit_mutable(&toplevel.events.map, nullptr);
}

void WindowManager::mapLayer(Layer& layer)
{
    if (layer.isFocusable())
    {
        focusLayer(layer);
    }

    wl_signal_emit_mutable(&layer.events.map, nullptr);
}

void WindowManager::unmapToplevel(Toplevel& toplevel)
{
    m_mappedToplevels.erase(toplevel.iter);

    if (m_focusState.toplevel == &toplevel)
    {
        m_focusState.toplevel = nullptr;

        // Focus the topmost toplevel if there is one
        if (!m_mappedToplevels.empty())
        {
            focusToplevel(*m_mappedToplevels.back());
        }
    }

    wl_signal_emit_mutable(&toplevel.events.unmap, nullptr);
}

void WindowManager::unmapLayer(Layer& layer)
{
    if (m_focusState.toplevel)
    {
        core.seat->setKeyboardFocus(m_focusState.toplevel->baseSurface());
    }

    if (m_focusState.layer == &layer)
    {
        m_focusState.layer = nullptr;
    }

    wl_signal_emit_mutable(&layer.events.unmap, nullptr);
}

void WindowManager::maximizeRequest(Toplevel& toplevel, const Output& output)
{
    if (toplevel.state.maximized)
    {
        return;
    }

    const auto& maxBox      = output.usableArea();
    const auto  toplevelPos = toplevel.position();
    const auto  toplevelGeo = toplevel.geometry();

    toplevel.restore.maximize.x = toplevelPos.x;
    toplevel.restore.maximize.y = toplevelPos.y;

    toplevel.restore.maximize.width  = toplevelGeo.width;
    toplevel.restore.maximize.height = toplevelGeo.height;

    toplevel.moveTo({maxBox.x, maxBox.y});
    toplevel.setSize(maxBox.width, maxBox.height);
    toplevel.setMaximized(true);
    toplevel.state.maximized = true;
}

void WindowManager::unmaximizeRequest(Toplevel& toplevel)
{
    if (!toplevel.state.maximized)
    {
        return;
    }

    toplevel.setSize(toplevel.restore.maximize.width, toplevel.restore.maximize.height);
    toplevel.moveTo({toplevel.restore.maximize.x, toplevel.restore.maximize.y});
    toplevel.setMaximized(false);
    toplevel.state.maximized = false;
}

void WindowManager::fullscreenRequest(Toplevel& toplevel, const Output& output)
{
    if (toplevel.state.fullscreen)
    {
        return;
    }

    const auto outputGeo   = output.layoutGeometry();
    const auto toplevelPos = toplevel.position();
    const auto toplevelGeo = toplevel.geometry();

    toplevel.restore.fullscreen.x = toplevelPos.x;
    toplevel.restore.fullscreen.y = toplevelPos.y;

    toplevel.restore.fullscreen.width  = toplevelGeo.width;
    toplevel.restore.fullscreen.height = toplevelGeo.height;

    toplevel.moveTo({outputGeo.x, outputGeo.y});
    toplevel.setSize(outputGeo.width, outputGeo.height);
    toplevel.setFullscreen(true);
    toplevel.state.fullscreen = true;

    if (++m_fullscreenCount; m_fullscreenCount == 1)
    {
        wlr_scene_node_set_enabled(&core.scene.shell.top->node, false);
    }
}

void WindowManager::unfullscreenRequest(Toplevel& toplevel)
{
    if (!toplevel.state.fullscreen)
    {
        return;
    }

    toplevel.setSize(toplevel.restore.fullscreen.width, toplevel.restore.fullscreen.height);
    toplevel.moveTo({toplevel.restore.fullscreen.x, toplevel.restore.fullscreen.y});
    toplevel.setFullscreen(false);
    toplevel.state.fullscreen = false;

    if (--m_fullscreenCount; m_fullscreenCount == 0)
    {
        wlr_scene_node_set_enabled(&core.scene.shell.top->node, true);
    }
}

void WindowManager::cycleToplevel()
{
    if (m_mappedToplevels.size() < 2)
    {
        return;
    }

    focusToplevel(*m_mappedToplevels.front());

    inputManager.state->rebasePointer();
}

void WindowManager::closeFocusedToplevel() const
{
    if (m_focusState.toplevel)
    {
        m_focusState.toplevel->close();
    }
}

}