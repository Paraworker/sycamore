#ifndef SYCAMORE_DRAG_ICON_H
#define SYCAMORE_DRAG_ICON_H

#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Seat;

class DragIcon
{
public:
    /**
     * @brief Create DragIcon
     * @return nullptr on failure
     */
    static DragIcon* create(wlr_drag_icon* handle, Seat& seat);

    void updatePosition() const;

    DragIcon(const DragIcon&) = delete;
    DragIcon(DragIcon&&) = delete;
    DragIcon& operator=(const DragIcon&) = delete;
    DragIcon& operator=(DragIcon&&) = delete;

private:
    DragIcon(wlr_drag_icon* handle, wlr_scene_tree* tree, Seat& seat);

    ~DragIcon() = default;

    void setPosition(const Point<int32_t>& pos) const
    {
        wlr_scene_node_set_position(&m_tree->node, pos.x, pos.y);
    }

private:
    wlr_drag_icon*  m_handle;
    wlr_scene_tree* m_tree;
    Seat&           m_seat;

private:
    Listener m_destroy;
};

class DragIconElement final : public SceneElement
{
public:
    DragIconElement(wlr_scene_node* node, DragIcon* icon)
        : SceneElement(SceneElement::DRAG_ICON, node)
        , m_icon(icon) {}

    ~DragIconElement() override = default;

    DragIcon& getIcon() const
    {
        return *m_icon;
    }

private:
    DragIcon* m_icon;
};

}

#endif //SYCAMORE_DRAG_ICON_H