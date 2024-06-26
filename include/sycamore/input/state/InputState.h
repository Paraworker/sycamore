#ifndef SYCAMORE_INPUT_STATE_H
#define SYCAMORE_INPUT_STATE_H

#include "sycamore/wlroots.h"

namespace sycamore
{

class InputState
{
public:
    virtual ~InputState() = default;

    virtual void onEnable() = 0;

    virtual void onDisable() = 0;

    virtual void onPointerButton(wlr_pointer_button_event* event) {};

    virtual void onPointerMotion(uint32_t timeMsec) {};

    virtual void onPointerAxis(wlr_pointer_axis_event* event) {};

    virtual void onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event) {};

    virtual void onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event) {};

    virtual void onPointerSwipeEnd(wlr_pointer_swipe_end_event* event) {};

    virtual void onPointerPinchBegin(wlr_pointer_pinch_begin_event* event) {};

    virtual void onPointerPinchUpdate(wlr_pointer_pinch_update_event* event) {};

    virtual void onPointerPinchEnd(wlr_pointer_pinch_end_event* event) {};

    virtual void onPointerHoldBegin(wlr_pointer_hold_begin_event* event) {};

    virtual void onPointerHoldEnd(wlr_pointer_hold_end_event* event) {};

    virtual void rebasePointer() {};

    virtual bool isInteractive() const = 0;
};

}

#endif //SYCAMORE_INPUT_STATE_H