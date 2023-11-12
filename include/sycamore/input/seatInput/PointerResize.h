#ifndef SYCAMORE_POINTER_RESIZE_H
#define SYCAMORE_POINTER_RESIZE_H

#include "sycamore/desktop/View.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "SeatInput.h"

NAMESPACE_SYCAMORE_BEGIN

class PointerResize : public SeatInput
{
public:
    PointerResize(View* view, uint32_t edges, Seat& seat);

    ~PointerResize() override;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    Type type() const override { return SeatInput::BINDING; }

private:
    View*         m_view;
    Listener      m_viewUnmap;
    uint32_t      m_edges;
    wlr_box       m_grabGeo;
    Point<double> m_delta;
    Seat&         m_seat;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POINTER_RESIZE_H