#ifndef SYCAMORE_DRAG_ICON_H
#define SYCAMORE_DRAG_ICON_H

#include "sycamore/defines.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

NAMESPACE_SYCAMORE_BEGIN

class Seat;

class DragIcon {
public:
    static void onCreate(wlr_drag_icon* handle, Seat& seat);

public:
    DragIcon(const DragIcon&) = delete;
    DragIcon(DragIcon&&) = delete;
    DragIcon& operator=(const DragIcon&) = delete;
    DragIcon& operator=(DragIcon&&) = delete;

    void updatePosition() const;

private:
    DragIcon(wlr_drag_icon* handle, wlr_scene_tree* tree, Seat& seat);
    ~DragIcon() = default;

    void setPosition(const Point<int32_t>& pos) const {
        wlr_scene_node_set_position(&m_tree->node, pos.x, pos.y);
    }

private:
    wlr_drag_icon*  m_handle;
    wlr_scene_tree* m_tree;
    Seat&           m_seat;

private:
    Listener m_destroy;
};

class DragIconElement final : public SceneElement {
public:
    DragIcon* getIcon() const { return m_icon; }

private:
    DragIconElement(wlr_scene_node* node, DragIcon* icon)
        : SceneElement(SceneElement::DRAG_ICON, node), m_icon(icon) {}

    ~DragIconElement() override = default;

private:
    friend class DragIcon;
    DragIcon* m_icon;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_DRAG_ICON_H