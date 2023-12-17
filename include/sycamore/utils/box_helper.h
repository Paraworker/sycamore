#ifndef SYCAMORE_BOX_HELPER_H
#define SYCAMORE_BOX_HELPER_H

#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

inline constexpr Point<int32_t> boxGetCenterCoords(const wlr_box& box)
{
    return {box.x + (box.width / 2), box.y + (box.height / 2)};
}

}

#endif //SYCAMORE_BOX_HELPER_H