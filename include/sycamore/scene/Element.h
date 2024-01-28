#ifndef SYCAMORE_SCENE_ELEMENT_H
#define SYCAMORE_SCENE_ELEMENT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore::scene
{

class Element
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

    /**
     * @brief Get derived element type
     */
    Type type() const
    {
        return m_type;
    }

protected:
    Element(Type type, wlr_scene_node* node) : m_type{type}
    {
        node->data = this;

        m_destroy.notify([this](auto)
        {
            delete this;
        });
        m_destroy.connect(node->events.destroy);
    }

    virtual ~Element() = default;

private:
    Type     m_type;
    Listener m_destroy;
};

}

#endif //SYCAMORE_SCENE_ELEMENT_H