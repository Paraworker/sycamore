#include "sycamore/desktop/Popup.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Popup::Popup(wlr_xdg_popup* handle, wlr_scene_tree* parent, std::shared_ptr<OwnerHandler> owner)
    : m_handle{handle}
    , m_tree{}
    , m_owner{std::move(owner)}
{
    // Create scene tree
    if (m_tree = wlr_scene_xdg_surface_create(parent, handle->base); !m_tree)
    {
        throw std::runtime_error{"Create scene tree for Popup failed!"};
    }

    // Create SceneElement;
    new PopupElement{&m_tree->node, *this};

    m_surfaceCommit.notify([this](auto)
    {
        if (m_handle->base->initial_commit)
        {
            m_owner->unconstrain(*this);
        }
    });
    m_surfaceCommit.connect(handle->base->surface->events.commit);

    m_reposition.notify([this](auto)
    {
        m_owner->unconstrain(*this);
    });
    m_reposition.connect(handle->events.reposition);

    m_newPopup.notify([this](void* data)
    {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, m_owner);
    });
    m_newPopup.connect(handle->base->events.new_popup);

    m_destroy.notify([this](auto)
    {
        delete this;
    });
    m_destroy.connect(handle->base->events.destroy);
}

Popup::~Popup() = default;

}