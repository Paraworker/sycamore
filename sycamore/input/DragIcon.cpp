#include "sycamore/input/DragIcon.h"
#include "sycamore/input/Seat.h"
#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

DragIcon* DragIcon::create(wlr_drag_icon* handle, Seat& seat) {
    auto tree = wlr_scene_drag_icon_create(Core::instance.scene->dragIcons, handle);
    if (!tree) {
        spdlog::error("Create scene tree for DragIcon failed!");
        return nullptr;
    }

    return new DragIcon(handle, tree, seat);
}

DragIcon::DragIcon(wlr_drag_icon* handle, wlr_scene_tree* tree, Seat& seat)
    : m_handle(handle), m_tree(tree), m_seat(seat) {
    m_destroy.set(&handle->events.destroy, [this](void*) { delete this; });

    updatePosition();

    // Create SceneElement
    new DragIconElement{&tree->node, this};
}

void DragIcon::updatePosition() const {
    switch (m_handle->drag->grab_type) {
        case WLR_DRAG_GRAB_KEYBOARD:
            return;
        case WLR_DRAG_GRAB_KEYBOARD_POINTER:
            setPosition(m_seat.getCursor().getPosition().into<int32_t>());
            break;
        case WLR_DRAG_GRAB_KEYBOARD_TOUCH:
            // TODO
            break;
    }
}

NAMESPACE_SYCAMORE_END