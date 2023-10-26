#include "sycamore/desktop/Popup.h"
#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Popup* Popup::create(wlr_xdg_popup* handle, wlr_scene_tree* parent) {
    // Create tree
    auto tree = wlr_scene_xdg_surface_create(parent, handle->base);
    if (!tree) {
        spdlog::error("Create scene tree for Popup failed");
        return nullptr;
    }

    // Create Popup
    return new Popup{handle, tree};
}

Popup::Popup(wlr_xdg_popup* handle, wlr_scene_tree* tree)
    : m_handle(handle), m_tree(tree) {
    m_newPopup.set(&handle->base->events.new_popup, [this](void* data) {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_tree);
    });

    m_destroy.set(&handle->base->events.destroy, [this](void*) {
        delete this;
    });

    // Create SceneElement;
    new PopupElement{&tree->node, this};
}

Popup::~Popup() = default;

NAMESPACE_SYCAMORE_END