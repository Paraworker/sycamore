#ifndef SYCAMORE_DEFAULT_INPUT_H
#define SYCAMORE_DEFAULT_INPUT_H

#include "SeatInput.h"

NAMESPACE_SYCAMORE_BEGIN

class DefaultInput : public SeatInput
{
public:
    explicit DefaultInput(Seat& seat) : m_seat{seat} {}

    ~DefaultInput() override = default;

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

    Type type() const override { return SeatInput::PASSTHROUGH; }

private:
    Seat& m_seat;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_DEFAULT_INPUT_H