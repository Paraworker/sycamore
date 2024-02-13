#include "sycamore/desktop/XdgToplevel.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/input/seatInput/PointerMove.h"
#include "sycamore/input/seatInput/PointerResize.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

struct XdgPopup : Popup::Handler
{
    XdgToplevel& toplevel;

    explicit XdgPopup(XdgToplevel& toplevel)
        : toplevel{toplevel}
    {}

    ~XdgPopup() override = default;

    void unconstrain(Popup& popup) override
    {
        if (auto output = toplevel.output(); output)
        {
            auto geo = toplevel.geometry();
            auto pos = toplevel.position();
            auto box = output->relativeGeometry();

            box.x = -pos.x + geo.x;
            box.y = -pos.y + geo.y;

            popup.unconstrainFromBox(box);
        }
    }
};

XdgToplevel::XdgToplevel(wlr_xdg_toplevel* toplevel)
    : Toplevel{toplevel->base->surface, wlr_scene_xdg_surface_create(core.scene.shell.toplevel, toplevel->base)}
    , m_toplevel{toplevel}
{
    // On creation, we only connect map, unmap, commit, destroy
    m_map = [this](auto)
    {
        const auto& req = m_toplevel->requested;

        m_newPopup.connect(m_toplevel->base->events.new_popup);
        m_move.connect(m_toplevel->events.request_move);
        m_resize.connect(m_toplevel->events.request_resize);
        m_fullscreen.connect(m_toplevel->events.request_fullscreen);
        m_maximize.connect(m_toplevel->events.request_maximize);
        m_minimize.connect(m_toplevel->events.request_minimize);

        windowManager.mapToplevel(*this, req.maximized, req.fullscreen);
    };
    m_map.connect(m_surface->events.map);

    m_unmap = [this](auto)
    {
        m_newPopup.disconnect();
        m_move.disconnect();
        m_resize.disconnect();
        m_fullscreen.disconnect();
        m_maximize.disconnect();
        m_minimize.disconnect();

        windowManager.unmapToplevel(*this);
    };
    m_unmap.connect(m_surface->events.unmap);

    m_commit = [this](auto)
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

        auto newGeo = geometry();

        // geometry changed
        if (memcmp(&m_lastGeo, &newGeo, sizeof(wlr_box)) != 0)
        {
            m_lastGeo = newGeo;
            core.seat->input->rebasePointer();
        }
    };
    m_commit.connect(m_surface->events.commit);

    m_destroy = [this](auto)
    {
        delete this;
    };
    m_destroy.connect(toplevel->base->events.destroy);

    // All listeners below are not connected util map

    m_newPopup = [this](void* data)
    {
        new Popup{static_cast<wlr_xdg_popup*>(data), m_tree, std::make_shared<XdgPopup>(*this)};
    };

    m_move = [this](auto)
    {
        if (!core.seat->bindingEnterCheck(*this))
        {
            return;
        }

        core.seat->setInput<PointerMove>(this, *core.seat);
    };

    m_resize = [this](void* data)
    {
        if (!core.seat->bindingEnterCheck(*this))
        {
            return;
        }

        auto event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
        core.seat->setInput<PointerResize>(this, event->edges, *core.seat);
    };

    m_fullscreen = [this](auto)
    {
        if (!m_toplevel->requested.fullscreen)
        {
            windowManager.unfullscreenRequest(*this);
            return;
        }

        if (auto o = output(); o)
        {
            windowManager.fullscreenRequest(*this, *o);
        }
    };

    m_maximize = [this](auto)
    {
        if (!m_toplevel->requested.maximized)
        {
            WindowManager::unmaximizeRequest(*this);
            return;
        }

        if (auto o = output(); o)
        {
            WindowManager::maximizeRequest(*this, *o);
        }
    };

    m_minimize = [](auto)
    {
        // TODO
    };
}

XdgToplevel::~XdgToplevel() = default;

uint32_t XdgToplevel::setMaximized(bool state)
{
    return wlr_xdg_toplevel_set_maximized(m_toplevel, state);
}

uint32_t XdgToplevel::setFullscreen(bool state)
{
    return wlr_xdg_toplevel_set_fullscreen(m_toplevel, state);
}

uint32_t XdgToplevel::setActivated(bool state)
{
    return wlr_xdg_toplevel_set_activated(m_toplevel, state);
}

uint32_t XdgToplevel::setResizing(bool state)
{
    return wlr_xdg_toplevel_set_resizing(m_toplevel, state);
}

uint32_t XdgToplevel::setSize(uint32_t width, uint32_t height)
{
    return wlr_xdg_toplevel_set_size(m_toplevel, width, height);
}

wlr_box XdgToplevel::geometry()
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