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
    // Create tree
    auto tree = wlr_scene_xdg_surface_create(Core::instance.scene->shell.toplevel, toplevel->base);
    if (!tree)
    {
        spdlog::error("Create scene tree for XdgToplevel failed!");
        return;
    }

    // Be destroyed by listener
    new XdgToplevel{toplevel, tree};
}

XdgToplevel::XdgToplevel(wlr_xdg_toplevel* toplevel, wlr_scene_tree* tree)
    : Toplevel{toplevel->base->surface, tree}, m_toplevel{toplevel}
{
    // On creation, we only connect destroy, map, unmap
    m_destroy.notify([this](auto)
    {
        delete this;
    });
    m_destroy.connect(toplevel->base->events.destroy);

    m_map.notify([this](auto)
    {
        ShellManager::instance.onToplevelMap(*this);

        // Connect signals
        m_commit.connect(m_surface->events.commit);
        m_newPopup.connect(m_toplevel->base->events.new_popup);
        m_move.connect(m_toplevel->events.request_move);
        m_resize.connect(m_toplevel->events.request_resize);
        m_fullscreen.connect(m_toplevel->events.request_fullscreen);
        m_maximize.connect(m_toplevel->events.request_maximize);
        m_minimize.connect(m_toplevel->events.request_minimize);

        // Layout stuff
        auto& requested = m_toplevel->requested;

        auto output = Core::instance.seat->cursor.atOutput();

        setToOutputCenter(output);

        if (requested.maximized)
        {
            ShellManager::maximizeRequest(*this, true, output);
        }

        if (requested.fullscreen)
        {
            ShellManager::instance.fullscreenRequest(*this, true, output);
        }

        wl_signal_emit_mutable(&events.map, nullptr);
    });
    m_map.connect(m_surface->events.map);

    m_unmap.notify([this](auto)
    {
        // Disconnect signals
        m_commit.disconnect();
        m_newPopup.disconnect();
        m_move.disconnect();
        m_resize.disconnect();
        m_fullscreen.disconnect();
        m_maximize.disconnect();
        m_minimize.disconnect();

        ShellManager::instance.onToplevelUnmap(*this);

        wl_signal_emit_mutable(&events.unmap, nullptr);
    });
    m_unmap.connect(m_surface->events.unmap);

    // All listeners below are not connected util map

    m_commit.notify([this](auto)
    {
        auto newGeometry = getGeometry();

        // geometry changed
        if (memcmp(&m_committedGeometry, &newGeometry, sizeof(wlr_box)) != 0)
        {
            m_committedGeometry = newGeometry;
            Core::instance.seat->input->rebasePointer();
        }
    });

    m_newPopup.notify([this](void* data)
    {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, std::make_shared<Popup::ToplevelHandler>(this));
    });

    m_move.notify([this](auto)
    {
        if (!Core::instance.seat->bindingEnterCheck(this))
        {
            return;
        }

        Core::instance.seat->setInput<PointerMove>(this, *Core::instance.seat);
    });

    m_resize.notify([this](void* data)
    {
        if (!Core::instance.seat->bindingEnterCheck(this))
        {
            return;
        }

        auto event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
        Core::instance.seat->setInput<PointerResize>(this, event->edges, *Core::instance.seat);
    });

    m_fullscreen.notify([this](auto)
    {
        auto&   requested = m_toplevel->requested;
        bool    state     = requested.fullscreen;
        Output* output    = nullptr;

        if (state)
        {
            // If there is an output provided, try to satisfy it
            if (auto fs = requested.fullscreen_output; fs && fs->data)
            {
                output = static_cast<Output*>(fs->data);
            }
            else
            {
                output = getOutput();
            }
        }

        ShellManager::instance.fullscreenRequest(*this, state, output);
    });

    m_maximize.notify([this](auto)
    {
        bool state = m_toplevel->requested.maximized;
        ShellManager::maximizeRequest(*this, state, state ? getOutput() : nullptr);
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