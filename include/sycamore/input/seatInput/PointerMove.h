#ifndef SYCAMORE_POINTER_MOVE_H
#define SYCAMORE_POINTER_MOVE_H

#include "sycamore/desktop/View.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/utils/Listener.h"
#include "SeatInput.h"

NAMESPACE_SYCAMORE_BEGIN

class PointerMove : public SeatInput {
public:
    PointerMove(View* view, Seat& seat);

    ~PointerMove() override;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    Type type() const override { return SeatInput::BINDING; }

private:
    View*         m_view;
    Listener      m_viewUnmap;
    Point<double> m_delta;
    Seat&         m_seat;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POINTER_MOVE_H