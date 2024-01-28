#ifndef SYCAMORE_DRAG_ICON_H
#define SYCAMORE_DRAG_ICON_H

#include "sycamore/scene/Element.h"
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
     */
    static void create(wlr_drag_icon* handle, Seat& seat);

    void updatePosition() const;

    DragIcon(const DragIcon&) = delete;
    DragIcon(DragIcon&&) = delete;
    DragIcon& operator=(const DragIcon&) = delete;
    DragIcon& operator=(DragIcon&&) = delete;

private:
    /**
     * @brief Constructor
     */
    DragIcon(wlr_drag_icon* handle, wlr_scene_tree* tree, Seat& seat);

    /**
     * @brief Destructor
     */
    ~DragIcon() = default;

    void setPosition(const Point<int32_t>& pos) const noexcept
    {
        wlr_scene_node_set_position(&m_tree->node, pos.x, pos.y);
    }

private:
    wlr_drag_icon*  m_handle;
    wlr_scene_tree* m_tree;
    Seat&           m_seat;

    Listener        m_destroy;
};

struct DragIconElement final : scene::Element
{
    DragIcon& icon;

    DragIconElement(wlr_scene_node* node, DragIcon& icon)
        : Element(DRAG_ICON, node)
        , icon(icon)
    {}

    ~DragIconElement() override = default;
};

}

#endif //SYCAMORE_DRAG_ICON_H