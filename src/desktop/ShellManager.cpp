#include "sycamore/desktop/ShellManager.h"

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

namespace sycamore
{

ShellManager::ShellManager() : m_focusState{}, m_fullscreenCount{0} {}

ShellManager::~ShellManager() = default;

void ShellManager::setFocus(Toplevel& toplevel)
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
    m_mappedToplevels.splice(m_mappedToplevels.end(), m_mappedToplevels, toplevel.iter());

    // Activate new toplevel
    toplevel.setActivated(true);

    if (!m_focusState.layer)
    {
        core.seat->setKeyboardFocus(toplevel.getBaseSurface());
    }

    m_focusState.toplevel = &toplevel;
}

void ShellManager::setFocus(Layer& layer)
{
    if (m_focusState.layer == &layer)
    {
        return;
    }

    m_focusState.layer = &layer;

    core.seat->setKeyboardFocus(layer.getBaseSurface());
}

void ShellManager::onToplevelMap(Toplevel& toplevel)
{
    toplevel.iter(m_mappedToplevels.emplace(m_mappedToplevels.end(), &toplevel));
    setFocus(toplevel);
}

void ShellManager::onToplevelUnmap(Toplevel& toplevel)
{
    m_mappedToplevels.erase(toplevel.iter());

    if (m_focusState.toplevel == &toplevel)
    {
        m_focusState.toplevel = nullptr;

        // Focus the topmost toplevel if there is one
        if (!m_mappedToplevels.empty())
        {
            setFocus(*m_mappedToplevels.back());
        }
    }
}

void ShellManager::onLayerMap(Layer& layer)
{
    if (layer.isFocusable())
    {
        setFocus(layer);
    }
}

void ShellManager::onLayerUnmap(Layer& layer)
{
    if (m_focusState.toplevel)
    {
        core.seat->setKeyboardFocus(m_focusState.toplevel->getBaseSurface());
    }

    if (m_focusState.layer == &layer)
    {
        m_focusState.layer = nullptr;
    }
}

void ShellManager::cycleToplevel()
{
    if (m_mappedToplevels.size() < 2)
    {
        return;
    }

    setFocus(*m_mappedToplevels.front());

    core.seat->input->rebasePointer();
}

void ShellManager::maximizeRequest(Toplevel& toplevel, const Output& output)
{
    if (toplevel.state().maximized)
    {
        return;
    }

    const auto& maxBox      = output.getUsableArea();
    const auto  toplevelPos = toplevel.getPosition();
    const auto  toplevelGeo = toplevel.getGeometry();

    toplevel.restore.maximize.x = toplevelPos.x;
    toplevel.restore.maximize.y = toplevelPos.y;

    toplevel.restore.maximize.width  = toplevelGeo.width;
    toplevel.restore.maximize.height = toplevelGeo.height;

    toplevel.moveTo({maxBox.x, maxBox.y});
    toplevel.setSize(maxBox.width, maxBox.height);
    toplevel.setMaximized(true);
    toplevel.state().maximized = true;
}

void ShellManager::unmaximizeRequest(Toplevel& toplevel)
{
    if (!toplevel.state().maximized)
    {
        return;
    }

    toplevel.setSize(toplevel.restore.maximize.width, toplevel.restore.maximize.height);
    toplevel.moveTo({toplevel.restore.maximize.x, toplevel.restore.maximize.y});
    toplevel.setMaximized(false);
    toplevel.state().maximized = false;
}

void ShellManager::fullscreenRequest(Toplevel& toplevel, const Output& output)
{
    if (toplevel.state().fullscreen)
    {
        return;
    }

    const auto outputGeo   = output.getLayoutGeometry();
    const auto toplevelPos = toplevel.getPosition();
    const auto toplevelGeo = toplevel.getGeometry();

    toplevel.restore.fullscreen.x = toplevelPos.x;
    toplevel.restore.fullscreen.y = toplevelPos.y;

    toplevel.restore.fullscreen.width  = toplevelGeo.width;
    toplevel.restore.fullscreen.height = toplevelGeo.height;

    toplevel.moveTo({outputGeo.x, outputGeo.y});
    toplevel.setSize(outputGeo.width, outputGeo.height);
    toplevel.setFullscreen(true);
    toplevel.state().fullscreen = true;

    if (++m_fullscreenCount; m_fullscreenCount == 1)
    {
        wlr_scene_node_set_enabled(&core.scene.shell.top->node, false);
    }
}

void ShellManager::unfullscreenRequest(Toplevel& toplevel)
{
    if (!toplevel.state().fullscreen)
    {
        return;
    }

    toplevel.setSize(toplevel.restore.fullscreen.width, toplevel.restore.fullscreen.height);
    toplevel.moveTo({toplevel.restore.fullscreen.x, toplevel.restore.fullscreen.y});
    toplevel.setFullscreen(false);
    toplevel.state().fullscreen = false;

    if (--m_fullscreenCount; m_fullscreenCount == 0)
    {
        wlr_scene_node_set_enabled(&core.scene.shell.top->node, true);
    }
}

}