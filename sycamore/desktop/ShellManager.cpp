#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

ShellManager ShellManager::instance{};

ShellManager::ShellManager()
{
    m_focusUnmap.toplevel.set([this](void*)
    {
        m_focusState.toplevel = nullptr;
        m_focusUnmap.toplevel.disconnect();
    });

    m_focusUnmap.layer.set([this](void*)
    {
        if (m_focusState.toplevel)
        {
            Core::instance.seat->setKeyboardFocus(m_focusState.toplevel->getBaseSurface());
        }

        m_focusState.layer = nullptr;
        m_focusUnmap.layer.disconnect();
    });
}

ShellManager::~ShellManager() = default;

void ShellManager::setFocus(Toplevel* toplevel)
{
    // Note: this function only deals with keyboard focus.
    if (m_focusState.toplevel == toplevel)
    {
        return;
    }

    if (m_focusState.toplevel)
    {
        /* Deactivate the previously focused toplevel. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret. */
        m_focusState.toplevel->setActivated(false);

        m_focusState.toplevel = nullptr;
        m_focusUnmap.toplevel.disconnect();
    }

    // Move the toplevel to the front
    toplevel->toFront();
    m_mappedToplevelList.reinsert(toplevel->link);

    // Activate the new toplevel
    toplevel->setActivated(true);

    if (!m_focusState.layer)
    {
        Core::instance.seat->setKeyboardFocus(toplevel->getBaseSurface());
    }

    m_focusState.toplevel = toplevel;
    m_focusUnmap.toplevel.connect(toplevel->events.unmap);
}

void ShellManager::setFocus(Layer* layer)
{
    if (m_focusState.layer == layer)
    {
        return;
    }

    m_focusState.layer = layer;
    m_focusUnmap.layer.connect(layer->events.unmap);

    Core::instance.seat->setKeyboardFocus(layer->getBaseSurface());
}

void ShellManager::addMappedToplevel(Toplevel* toplevel)
{
    m_mappedToplevelList.add(toplevel->link);
}

void ShellManager::removeMappedToplevel(Toplevel* toplevel)
{
    m_mappedToplevelList.remove(toplevel->link);
}

void ShellManager::maximizeRequest(Toplevel* toplevel, bool state, Output* output)
{
    if (state == toplevel->state().maximized)
    {
        return;
    }

    if (!state)
    {
        // Restore from maximized mode
        toplevel->setMaximized(state);
        toplevel->setSize(toplevel->restore.maximize.width, toplevel->restore.maximize.height);
        toplevel->moveTo({toplevel->restore.maximize.x, toplevel->restore.maximize.y});
        toplevel->state().maximized = state;
        return;
    }

    // Set to maximized mode
    if (!output)
    {
        return;
    }

    auto& maxArea     = output->getUsableArea();
    auto  toplevelPos = toplevel->getPosition();
    auto  toplevelGeo = toplevel->getGeometry();

    toplevel->restore.maximize.x = toplevelPos.x;
    toplevel->restore.maximize.y = toplevelPos.y;

    toplevel->restore.maximize.width  = toplevelGeo.width;
    toplevel->restore.maximize.height = toplevelGeo.height;

    toplevel->moveTo({maxArea.x, maxArea.y});
    toplevel->setSize(maxArea.width, maxArea.height);
    toplevel->setMaximized(state);
    toplevel->state().maximized = state;
}

void ShellManager::fullscreenRequest(Toplevel* toplevel, bool state, Output* output)
{
    if (state == toplevel->state().fullscreen)
    {
        return;
    }

    if (!state)
    {
        // Restore from fullscreen mode
        wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, true);

        toplevel->setSize(toplevel->restore.fullscreen.width, toplevel->restore.fullscreen.height);
        toplevel->moveTo({toplevel->restore.fullscreen.x, toplevel->restore.fullscreen.y});
        toplevel->setFullscreen(state);
        toplevel->state().fullscreen = state;
        return;
    }

    // Set to fullscreen mode
    if (!output)
    {
        return;
    }

    auto outputGeo   = output->getLayoutGeometry();
    auto toplevelPos = toplevel->getPosition();
    auto toplevelGeo = toplevel->getGeometry();

    toplevel->restore.fullscreen.x = toplevelPos.x;
    toplevel->restore.fullscreen.y = toplevelPos.y;

    toplevel->restore.fullscreen.width  = toplevelGeo.width;
    toplevel->restore.fullscreen.height = toplevelGeo.height;

    wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, false);

    toplevel->moveTo({outputGeo.x, outputGeo.y});
    toplevel->setSize(outputGeo.width, outputGeo.height);
    toplevel->setFullscreen(state);
    toplevel->state().fullscreen = state;
}

NAMESPACE_SYCAMORE_END