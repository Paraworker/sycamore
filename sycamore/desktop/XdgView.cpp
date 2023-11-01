#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/desktop/XdgView.h"
#include "sycamore/input/seatInput/PointerMove.h"
#include "sycamore/input/seatInput/PointerResize.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

XdgView* XdgView::create(wlr_xdg_toplevel* toplevel) {
    // Create tree
    auto tree = wlr_scene_xdg_surface_create(Core::instance.scene->shell.view, toplevel->base);
    if (!tree) {
        spdlog::error("Create scene tree for XdgView failed!");
        return nullptr;
    }

    // Create XdgView
    return new XdgView{toplevel, tree};
}

XdgView::XdgView(wlr_xdg_toplevel* toplevel, wlr_scene_tree* tree)
    : View(toplevel->base->surface, tree)
    , m_toplevel(toplevel) {
    // On creation, we only connect destroy, map, unmap
    m_destroy.set(&m_surface->events.destroy, [this](void*) {
        // emit signal before destruction
        wl_signal_emit_mutable(&events.destroy, nullptr);
        delete this;
    });

    m_map.set(&m_surface->events.map, [this](void*) {
        // Add to list
        ShellManager::instance.addMappedView(this);

        // Connect signals
        m_commit.connect(&m_surface->events.commit);
        m_newPopup.connect(&m_toplevel->base->events.new_popup);
        m_move.connect(&m_toplevel->events.request_move);
        m_resize.connect(&m_toplevel->events.request_resize);
        m_fullscreen.connect(&m_toplevel->events.request_fullscreen);
        m_maximize.connect(&m_toplevel->events.request_maximize);
        m_minimize.connect(&m_toplevel->events.request_minimize);

        // Layout stuff
        auto& requested = m_toplevel->requested;

        auto output = Core::instance.seat->getCursor().atOutput();

        setToOutputCenter(output);

        if (requested.maximized) {
            ShellManager::maximizeRequest(this, true, output);
        }

        if (requested.fullscreen) {
            ShellManager::fullscreenRequest(this, true, output);
        }

        // Focus it
        ShellManager::instance.setFocus(this);

        // emit signal
        wl_signal_emit_mutable(&events.map, nullptr);
    });

    m_unmap.set(&m_surface->events.unmap, [this](void*) {
        // Disconnect signals
        m_commit.disconnect();
        m_newPopup.disconnect();
        m_move.disconnect();
        m_resize.disconnect();
        m_fullscreen.disconnect();
        m_maximize.disconnect();
        m_minimize.disconnect();

        ShellManager::instance.removeMappedView(this);

        // emit signal
        wl_signal_emit_mutable(&events.unmap, nullptr);
    });

    // All listeners below are not connected util map

    m_commit.set([this](void*) {
        auto newGeometry = getGeometry();

        // geometry changed
        if (memcmp(&m_committedGeometry, &newGeometry, sizeof(wlr_box)) != 0) {
            m_committedGeometry = newGeometry;
            Core::instance.seat->getInput().rebasePointer();
        }
    });

    m_newPopup.set([this](void* data) {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, std::make_shared<Popup::ViewHandler>(this));
    });

    m_move.set([this](void*) {
        if (!Core::instance.seat->bindingEnterCheck(this)) {
            return;
        }

        Core::instance.seat->setInput(new PointerMove(this, *Core::instance.seat));
    });

    m_resize.set([this](void* data) {
        if (!Core::instance.seat->bindingEnterCheck(this)) {
            return;
        }

        auto event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
        Core::instance.seat->setInput(new PointerResize(this, event->edges, *Core::instance.seat));
    });

    m_fullscreen.set([this](void*) {
        auto& requested = m_toplevel->requested;

        if (!requested.fullscreen) {
            ShellManager::fullscreenRequest(this, false, nullptr);
            return;
        }

        // If there is an output provided, try to satisfy it
        auto output = requested.fullscreen_output;
        if (output && output->data) {
            ShellManager::fullscreenRequest(this, true, static_cast<Output*>(output->data));
            return;
        }

        ShellManager::fullscreenRequest(this, true, getOutput());
    });

    m_maximize.set([this](void*) {
        if (!m_toplevel->requested.maximized) {
            ShellManager::maximizeRequest(this, false, nullptr);
            return;
        }

        ShellManager::maximizeRequest(this, true, getOutput());
    });

    m_minimize.set([](void*) {
        // TODO
    });
}

XdgView::~XdgView() = default;

uint32_t XdgView::setMaximized(bool maximized) {
    return wlr_xdg_toplevel_set_maximized(m_toplevel, maximized);
}

uint32_t XdgView::setFullscreen(bool fullscreen) {
    return wlr_xdg_toplevel_set_fullscreen(m_toplevel, fullscreen);
}

uint32_t XdgView::setActivated(bool activated) {
    return wlr_xdg_toplevel_set_activated(m_toplevel, activated);
}

uint32_t XdgView::setResizing(bool resizing) {
    return wlr_xdg_toplevel_set_resizing(m_toplevel, resizing);
}

uint32_t XdgView::setSize(uint32_t width, uint32_t height) {
    return wlr_xdg_toplevel_set_size(m_toplevel, width, height);
}

wlr_box XdgView::getGeometry() {
    wlr_box box;
    wlr_xdg_surface_get_geometry(m_toplevel->base, &box);
    return box;
}

void XdgView::close() {
    wlr_xdg_toplevel_send_close(m_toplevel);
}

NAMESPACE_SYCAMORE_END