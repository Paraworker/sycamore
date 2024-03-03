#ifndef SYCAMORE_PASSTHROUGH_H
#define SYCAMORE_PASSTHROUGH_H

#include "InputState.h"

namespace sycamore
{

class Passthrough final : public InputState
{
public:
    Passthrough() = default;

    ~Passthrough() override = default;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    void onPointerAxis(wlr_pointer_axis_event* event) override;

    void onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event) override;

    void onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event) override;

    void onPointerSwipeEnd(wlr_pointer_swipe_end_event* event) override;

    void onPointerPinchBegin(wlr_pointer_pinch_begin_event* event) override;

    void onPointerPinchUpdate(wlr_pointer_pinch_update_event* event) override;

    void onPointerPinchEnd(wlr_pointer_pinch_end_event* event) override;

    void onPointerHoldBegin(wlr_pointer_hold_begin_event* event) override;

    void onPointerHoldEnd(wlr_pointer_hold_end_event* event) override;

    void rebasePointer() override;

    bool isInteractive() const override;
};

}

#endif //SYCAMORE_PASSTHROUGH_H