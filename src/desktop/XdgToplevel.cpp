#include "sycamore/desktop/XdgToplevel.h"

#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/input/seatInput/PointerMove.h"
#include "sycamore/input/seatInput/PointerResize.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

void XdgToplevel::create(wlr_xdg_toplevel* toplevel)
{
    auto tree = wlr_scene_xdg_surface_create(core.scene.shell.toplevel, toplevel->base);
    if (!tree)
    {
        spdlog::error("Create scene tree for XdgToplevel failed!");
        return;
    }

    new XdgToplevel{toplevel, tree};
}

XdgToplevel::XdgToplevel(wlr_xdg_toplevel* toplevel, wlr_scene_tree* tree)
    : Toplevel{toplevel->base->surface, tree}, m_toplevel{toplevel}
{
    // On creation, we only connect map, unmap, commit, destroy
    m_map.notify([this](auto)
    {
        shellManager.onToplevelMap(*this);

        // Connect signals
        m_newPopup.connect(m_toplevel->base->events.new_popup);
        m_move.connect(m_toplevel->events.request_move);
        m_resize.connect(m_toplevel->events.request_resize);
        m_fullscreen.connect(m_toplevel->events.request_fullscreen);
        m_maximize.connect(m_toplevel->events.request_maximize);
        m_minimize.connect(m_toplevel->events.request_minimize);

        // Layout stuff
        auto output = core.seat->cursor.atOutput();

        if (output)
        {
            setToOutputCenter(*output);

            const auto& requested = m_toplevel->requested;

            if (requested.maximized)
            {
                ShellManager::maximizeRequest(*this, *output);
            }

            if (requested.fullscreen)
            {
                shellManager.fullscreenRequest(*this, *output);
            }
        }

        wl_signal_emit_mutable(&events.map, nullptr);
    });
    m_map.connect(m_surface->events.map);

    m_unmap.notify([this](auto)
    {
        // Disconnect signals
        m_newPopup.disconnect();
        m_move.disconnect();
        m_resize.disconnect();
        m_fullscreen.disconnect();
        m_maximize.disconnect();
        m_minimize.disconnect();

        shellManager.onToplevelUnmap(*this);

        wl_signal_emit_mutable(&events.unmap, nullptr);
    });
    m_unmap.connect(m_surface->events.unmap);

    m_commit.notify([this](auto)
    {
        if (m_toplevel->base->initial_commit)
        {
            // Configures the xdg_toplevel with 0,0 size
            // to let the client pick the dimensions itself
            wlr_xdg_toplevel_set_size(m_toplevel, 0, 0);
            return;
        }

        if (!isMapped())
        {
            return;
        }

        auto newGeometry = getGeometry();

        // geometry changed
        if (memcmp(&m_committedGeometry, &newGeometry, sizeof(wlr_box)) != 0)
        {
            m_committedGeometry = newGeometry;
            core.seat->input->rebasePointer();
        }
    });
    m_commit.connect(m_surface->events.commit);

    m_destroy.notify([this](auto)
    {
        delete this;
    });
    m_destroy.connect(toplevel->base->events.destroy);

    // All listeners below are not connected util map

    m_newPopup.notify([this](void* data)
    {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, std::make_shared<Popup::ToplevelHandler>(*this));
    });

    m_move.notify([this](auto)
    {
        if (!core.seat->bindingEnterCheck(this))
        {
            return;
        }

        core.seat->setInput<PointerMove>(this, *core.seat);
    });

    m_resize.notify([this](void* data)
    {
        if (!core.seat->bindingEnterCheck(this))
        {
            return;
        }

        auto event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
        core.seat->setInput<PointerResize>(this, event->edges, *core.seat);
    });

    m_fullscreen.notify([this](auto)
    {
        if (!m_toplevel->requested.fullscreen)
        {
            shellManager.unfullscreenRequest(*this);
            return;
        }

        if (auto output = getOutput(); output)
        {
            shellManager.fullscreenRequest(*this, *output);
        }
    });

    m_maximize.notify([this](auto)
    {
        if (!m_toplevel->requested.maximized)
        {
            ShellManager::unmaximizeRequest(*this);
            return;
        }

        if (auto output = getOutput(); output)
        {
            ShellManager::maximizeRequest(*this, *output);
        }
    });

    m_minimize.notify([](auto)
    {
        // TODO
    });
}

XdgToplevel::~XdgToplevel() = default;

uint32_t XdgToplevel::setMaximized(bool maximized)
{
    return wlr_xdg_toplevel_set_maximized(m_toplevel, maximized);
}

uint32_t XdgToplevel::setFullscreen(bool fullscreen)
{
    return wlr_xdg_toplevel_set_fullscreen(m_toplevel, fullscreen);
}

uint32_t XdgToplevel::setActivated(bool activated)
{
    return wlr_xdg_toplevel_set_activated(m_toplevel, activated);
}

uint32_t XdgToplevel::setResizing(bool resizing)
{
    return wlr_xdg_toplevel_set_resizing(m_toplevel, resizing);
}

uint32_t XdgToplevel::setSize(uint32_t width, uint32_t height)
{
    return wlr_xdg_toplevel_set_size(m_toplevel, width, height);
}

wlr_box XdgToplevel::getGeometry()
{
    wlr_box box{};
    wlr_xdg_surface_get_geometry(m_toplevel->base, &box);
    return box;
}

void XdgToplevel::close()
{
    wlr_xdg_toplevel_send_close(m_toplevel);
}

}