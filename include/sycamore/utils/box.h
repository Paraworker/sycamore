#ifndef SYCAMORE_BOX_H
#define SYCAMORE_BOX_H

#include "sycamore/defines.h"
#include "sycamore/wlroots.h"
#include "sycamore/utils/Point.h"

NAMESPACE_SYCAMORE_BEGIN

inline constexpr Point<int32_t> boxGetCenterCoords(const wlr_box& box)
{
    return {box.x + (box.width / 2), box.y + (box.height / 2)};
}

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_BOX_H