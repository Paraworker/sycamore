#ifndef SYCAMORE_SCENE_ELEMENT_H
#define SYCAMORE_SCENE_ELEMENT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class SceneElement
{
public:
    enum Type
    {
        ROOT,
        TOPLEVEL,
        LAYER,
        POPUP,
        DRAG_ICON,
    };

public:
    Type type() const
    {
        return m_type;
    }

protected:
    SceneElement(Type type, wlr_scene_node* node) : m_type{type}
    {
        node->data = this;

        m_destroy.connect(node->events.destroy);
        m_destroy.set([this](auto)
        {
            delete this;
        });
    }

    virtual ~SceneElement() = default;

protected:
    Type     m_type;
    Listener m_destroy;
};

}

#endif //SYCAMORE_SCENE_ELEMENT_H