#ifndef SYCAMORE_SCENE_ELEMENT_H
#define SYCAMORE_SCENE_ELEMENT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore::scene
{

struct Element
{
    enum Kind
    {
        ROOT,
        TOPLEVEL,
        LAYER,
        POPUP,
        DRAG_ICON,
    };

    Kind kind;
};

}

#endif //SYCAMORE_SCENE_ELEMENT_H