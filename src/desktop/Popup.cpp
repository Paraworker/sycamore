#include "sycamore/desktop/Popup.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

void Popup::create(wlr_xdg_popup* handle, wlr_scene_tree* parentTree, const OwnerHandler::SPtr& owner)
{
    // Create tree
    auto tree = wlr_scene_xdg_surface_create(parentTree, handle->base);
    if (!tree)
    {
        spdlog::error("Create scene tree for Popup failed");
        return;
    }

    // Be destroyed by listener
    new Popup{handle, tree, owner};
}

Popup::Popup(wlr_xdg_popup* handle, wlr_scene_tree* tree, OwnerHandler::SPtr owner)
    : m_handle{handle}, m_tree{tree}, m_owner{std::move(owner)}
{
    m_surfaceCommit
    .connect(handle->base->surface->events.commit)
    .set([this](auto)
    {
        if (m_handle->base->initial_commit)
        {
            m_owner->unconstrain(*this);
        }
    });

    m_reposition
    .connect(handle->events.reposition)
    .set([this](auto)
    {
        m_owner->unconstrain(*this);
    });

    m_newPopup
    .connect(handle->base->events.new_popup)
    .set([this](void* data)
    {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, m_owner);
    });

    m_destroy
    .connect(handle->base->events.destroy)
    .set([this](auto)
    {
        delete this;
    });

    // Create SceneElement;
    new PopupElement{&tree->node, this};
}

Popup::~Popup() = default;

}