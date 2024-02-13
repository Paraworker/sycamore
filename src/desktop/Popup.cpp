#include "sycamore/desktop/Popup.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Popup::Popup(wlr_xdg_popup* handle, wlr_scene_tree* parent, std::shared_ptr<Handler> handler)
    : m_handle{handle}
    , m_tree{wlr_scene_xdg_surface_create(parent, handle->base)}
    , m_handler{std::move(handler)}
{
    new PopupElement{m_tree->node, *this};

    m_surfaceCommit = [this](auto)
    {
        if (m_handle->base->initial_commit)
        {
            m_handler->unconstrain(*this);
        }
    };
    m_surfaceCommit.connect(handle->base->surface->events.commit);

    m_reposition = [this](auto)
    {
        m_handler->unconstrain(*this);
    };
    m_reposition.connect(handle->events.reposition);

    m_newPopup = [this](void* data)
    {
        new Popup{static_cast<wlr_xdg_popup*>(data), m_tree, m_handler};
    };
    m_newPopup.connect(handle->base->events.new_popup);

    m_destroy = [this](auto)
    {
        delete this;
    };
    m_destroy.connect(handle->base->events.destroy);
}

Popup::~Popup() = default;

}