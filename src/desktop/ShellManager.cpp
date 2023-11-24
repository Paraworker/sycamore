#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

ShellManager ShellManager::instance{};

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
        /* Deactivate the previously focused toplevel. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret. */
        m_focusState.toplevel->setActivated(false);
    }

    // Move the toplevel to the front
    toplevel.toFront();
    m_mappedToplevelList.reinsert(toplevel.link);

    // Activate the new toplevel
    toplevel.setActivated(true);

    if (!m_focusState.layer)
    {
        Core::instance.seat->setKeyboardFocus(toplevel.getBaseSurface());
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

    Core::instance.seat->setKeyboardFocus(layer.getBaseSurface());
}

void ShellManager::onToplevelMap(Toplevel& toplevel)
{
    m_mappedToplevelList.add(toplevel.link);
    setFocus(toplevel);
}

void ShellManager::onToplevelUnmap(Toplevel& toplevel)
{
    m_mappedToplevelList.remove(toplevel.link);

    if (m_focusState.toplevel == &toplevel)
    {
        m_focusState.toplevel = nullptr;

        // Focus the topmost toplevel if there is one
        if (m_mappedToplevelList.size() > 0)
        {
            Toplevel* newFocus = wl_container_of(m_mappedToplevelList.getHandle().next, newFocus, link);
            setFocus(*newFocus);
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
        Core::instance.seat->setKeyboardFocus(m_focusState.toplevel->getBaseSurface());
    }

    if (m_focusState.layer == &layer)
    {
        m_focusState.layer = nullptr;
    }
}

void ShellManager::cycleToplevel()
{
    if (m_mappedToplevelList.size() < 2)
    {
        return;
    }

    Toplevel* next = wl_container_of(m_mappedToplevelList.getHandle().prev, next, link);
    setFocus(*next);

    Core::instance.seat->getInput().rebasePointer();
}

void ShellManager::maximizeRequest(Toplevel& toplevel, bool state, Output* output)
{
    if (state == toplevel.state().maximized)
    {
        return;
    }

    if (!state)
    {
        // Restore from maximized mode
        toplevel.setMaximized(state);
        toplevel.setSize(toplevel.restore.maximize.width, toplevel.restore.maximize.height);
        toplevel.moveTo({toplevel.restore.maximize.x, toplevel.restore.maximize.y});
        toplevel.state().maximized = state;
        return;
    }

    // Set to maximized mode
    if (!output)
    {
        return;
    }

    auto& maxBox      = output->getUsableArea();
    auto  toplevelPos = toplevel.getPosition();
    auto  toplevelGeo = toplevel.getGeometry();

    toplevel.restore.maximize.x = toplevelPos.x;
    toplevel.restore.maximize.y = toplevelPos.y;

    toplevel.restore.maximize.width  = toplevelGeo.width;
    toplevel.restore.maximize.height = toplevelGeo.height;

    toplevel.moveTo({maxBox.x, maxBox.y});
    toplevel.setSize(maxBox.width, maxBox.height);
    toplevel.setMaximized(state);
    toplevel.state().maximized = state;
}

void ShellManager::fullscreenRequest(Toplevel& toplevel, bool state, Output* output)
{
    if (state == toplevel.state().fullscreen)
    {
        return;
    }

    if (!state)
    {
        // Restore from fullscreen mode
        toplevel.setSize(toplevel.restore.fullscreen.width, toplevel.restore.fullscreen.height);
        toplevel.moveTo({toplevel.restore.fullscreen.x, toplevel.restore.fullscreen.y});
        toplevel.setFullscreen(state);
        toplevel.state().fullscreen = state;

        if (--m_fullscreenCount; m_fullscreenCount == 0)
        {
            wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, true);
        }

        return;
    }

    // Set to fullscreen mode
    if (!output)
    {
        return;
    }

    auto outputGeo   = output->getLayoutGeometry();
    auto toplevelPos = toplevel.getPosition();
    auto toplevelGeo = toplevel.getGeometry();

    toplevel.restore.fullscreen.x = toplevelPos.x;
    toplevel.restore.fullscreen.y = toplevelPos.y;

    toplevel.restore.fullscreen.width  = toplevelGeo.width;
    toplevel.restore.fullscreen.height = toplevelGeo.height;

    toplevel.moveTo({outputGeo.x, outputGeo.y});
    toplevel.setSize(outputGeo.width, outputGeo.height);
    toplevel.setFullscreen(state);
    toplevel.state().fullscreen = state;

    if (++m_fullscreenCount; m_fullscreenCount == 1)
    {
        wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, false);
    }
}

NAMESPACE_SYCAMORE_END