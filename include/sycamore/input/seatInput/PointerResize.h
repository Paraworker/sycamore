#ifndef SYCAMORE_POINTER_RESIZE_H
#define SYCAMORE_POINTER_RESIZE_H

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/Seat.h"
#include "SeatInput.h"

namespace sycamore
{

class PointerResize : public SeatInput
{
public:
    PointerResize(Toplevel* toplevel, uint32_t edges, Seat& seat);

    ~PointerResize() override;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    Type type() const override
    {
        return BINDING;
    }

private:
    Toplevel*     m_toplevel;
    Listener      m_toplevelUnmap;
    uint32_t      m_edges;
    wlr_box       m_grabGeo;
    Point<double> m_delta;
    Seat&         m_seat;
};

}

#endif //SYCAMORE_POINTER_RESIZE_H