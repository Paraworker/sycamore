#ifndef SYCAMORE_DRAG_ICON_H
#define SYCAMORE_DRAG_ICON_H

#include "sycamore/scene/Element.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

namespace sycamore
{

class DragIcon;

struct DragIconElement final : scene::Element
{
    DragIcon& icon;
    Listener  destroy;

    DragIconElement(wlr_scene_node& node, DragIcon& icon)
        : scene::Element{DRAG_ICON}, icon{icon}
    {
        // attach node
        node.data = this;

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(node.events.destroy);
    }

    ~DragIconElement() = default;
};

class DragIcon
{
public:
    explicit DragIcon(wlr_drag_icon* handle)
        : m_handle{handle}
        , m_tree{wlr_scene_drag_icon_create(core.scene.dragIcons, handle)}
    {
        new DragIconElement{m_tree->node, *this};

        m_destroy = [this](auto)
        {
            delete this;
        };
        m_destroy.connect(m_handle->events.destroy);
    }

    ~DragIcon() = default;

    void setPosition(const Point<int32_t>& pos) const
    {
        wlr_scene_node_set_position(&m_tree->node, pos.x, pos.y);
    }

    auto grabType() const
    {
        return m_handle->drag->grab_type;
    }

    DragIcon(const DragIcon&) = delete;
    DragIcon(DragIcon&&) = delete;
    DragIcon& operator=(const DragIcon&) = delete;
    DragIcon& operator=(DragIcon&&) = delete;

private:
    wlr_drag_icon*  m_handle;
    wlr_scene_tree* m_tree;

    Listener        m_destroy;
};

}

#endif //SYCAMORE_DRAG_ICON_H