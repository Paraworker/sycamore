#include "sycamore/desktop/Popup.h"
#include "sycamore/output/Output.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Popup* Popup::create(wlr_xdg_popup* handle, wlr_scene_tree* parentTree, const OwnerHandler::SPtr& owner) {
    // Create tree
    auto tree = wlr_scene_xdg_surface_create(parentTree, handle->base);
    if (!tree) {
        spdlog::error("Create scene tree for Popup failed");
        return nullptr;
    }

    // Create Popup
    return new Popup{handle, tree, owner};
}

Popup::Popup(wlr_xdg_popup* handle, wlr_scene_tree* tree, OwnerHandler::SPtr owner)
    : m_handle(handle), m_tree(tree), m_owner(std::move(owner)) {
    m_reposition
    .connect(handle->events.reposition)
    .set([this](void*) {
        m_owner->unconstrain(this);
    });

    m_newPopup
    .connect(handle->base->events.new_popup)
    .set([this](void* data) {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree, m_owner);
    });

    m_destroy
    .connect(handle->base->events.destroy)
    .set([this](void*) { delete this; });

    m_owner->unconstrain(this);

    // Create SceneElement;
    new PopupElement{&tree->node, this};
}

Popup::~Popup() = default;

NAMESPACE_SYCAMORE_END