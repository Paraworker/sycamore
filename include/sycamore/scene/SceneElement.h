#ifndef SYCAMORE_SCENE_ELEMENT_H
#define SYCAMORE_SCENE_ELEMENT_H

#include "sycamore/defines.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

NAMESPACE_SYCAMORE_BEGIN

class SceneElement {
public:
    enum Type {
        ROOT,
        VIEW,
        LAYER,
        POPUP,
        DRAG_ICON,
    };

public:
    Type type() const { return m_type; }

protected:
    SceneElement(Type type, wlr_scene_node* node) : m_type(type) {
        node->data = this;

        m_destroy.set(&node->events.destroy, [this](void*) {
            delete this;
        });
    }

    virtual ~SceneElement() = default;

protected:
    Type     m_type;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SCENE_ELEMENT_H