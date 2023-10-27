#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/View.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

ShellManager ShellManager::instance{};

ShellManager::ShellManager() {
    m_focusUnmap.view.set([this](void*) {
        m_focusState.view = nullptr;
        m_focusUnmap.view.disconnect();
    });

    m_focusUnmap.layer.set([this](void*) {
        if (m_focusState.view) {
            Core::instance.seat->setKeyboardFocus(m_focusState.view->getBaseSurface());
        }

        m_focusState.layer = nullptr;
        m_focusUnmap.layer.disconnect();
    });
}

ShellManager::~ShellManager() = default;

void ShellManager::setFocus(View* view) {
    // Note: this function only deals with keyboard focus.
    if (m_focusState.view == view) {
        return;
    }

    if (m_focusState.view) {
        /* Deactivate the previously focused view. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret. */
        m_focusState.view->setActivated(false);

        m_focusState.view = nullptr;
        m_focusUnmap.view.disconnect();
    }

    // Move the view to the front
    view->toFront();
    m_mappedViewList.reinsert(view->link);

    // Activate the new view
    view->setActivated(true);

    if (!m_focusState.layer) {
        Core::instance.seat->setKeyboardFocus(view->getBaseSurface());
    }

    m_focusState.view = view;
    m_focusUnmap.view.connect(&view->events.unmap);
}

void ShellManager::setFocus(Layer* layer) {
    if (m_focusState.layer == layer) {
        return;
    }

    m_focusState.layer = layer;
    m_focusUnmap.layer.connect(&layer->events.unmap);

    Core::instance.seat->setKeyboardFocus(layer->getBaseSurface());
}

void ShellManager::addMappedView(View* view) {
    m_mappedViewList.add(view->link);
}

void ShellManager::removeMappedView(View* view) {
    m_mappedViewList.remove(view->link);
}

void ShellManager::maximizeRequest(View* view, bool state, Output* output) {
    if (state == view->state().maximized) {
        return;
    }

    if (!state) {
        // Restore from maximized mode
        view->setMaximized(state);
        view->setSize(view->restore.maximize.width, view->restore.maximize.height);
        view->moveTo({view->restore.maximize.x, view->restore.maximize.y});
        view->state().maximized = state;
        return;
    }

    // Set to maximized mode
    if (!output) {
        return;
    }

    auto& maxArea = output->getUsableArea();
    auto  viewPos = view->getPosition();
    auto  viewGeo = view->getGeometry();

    view->restore.maximize.x = viewPos.x;
    view->restore.maximize.y = viewPos.y;

    view->restore.maximize.width  = viewGeo.width;
    view->restore.maximize.height = viewGeo.height;

    view->moveTo({maxArea.x, maxArea.y});
    view->setSize(maxArea.width, maxArea.height);
    view->setMaximized(state);
    view->state().maximized = state;
}

void ShellManager::fullscreenRequest(View *view, bool state, Output *output) {
    if (state == view->state().fullscreen) {
        return;
    }

    if (!state) {
        // Restore from fullscreen mode
        wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, true);

        view->setSize(view->restore.fullscreen.width, view->restore.fullscreen.height);
        view->moveTo({view->restore.fullscreen.x, view->restore.fullscreen.y});
        view->setFullscreen(state);
        view->state().fullscreen = state;
        return;
    }

    // Set to fullscreen mode
    if (!output) {
        return;
    }

    auto outputGeo  = output->getLayoutGeometry();
    auto viewCoords = view->getPosition();
    auto viewGeo    = view->getGeometry();

    view->restore.fullscreen.x = viewCoords.x;
    view->restore.fullscreen.y = viewCoords.y;

    view->restore.fullscreen.width  = viewGeo.width;
    view->restore.fullscreen.height = viewGeo.height;

    wlr_scene_node_set_enabled(&Core::instance.scene->shell.top->node, false);

    view->moveTo({outputGeo.x, outputGeo.y});
    view->setSize(outputGeo.width, outputGeo.height);
    view->setFullscreen(state);
    view->state().fullscreen = state;
}

NAMESPACE_SYCAMORE_END