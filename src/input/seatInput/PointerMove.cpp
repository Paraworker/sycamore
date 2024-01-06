#include "sycamore/input/seatInput/PointerMove.h"

#include "sycamore/input/seatInput/DefaultInput.h"

namespace sycamore
{

PointerMove::PointerMove(Toplevel* toplevel, Seat& seat)
    : m_toplevel{toplevel}
    , m_delta{seat.cursor.getPosition() - toplevel->getPosition().into<double>()}
    , m_seat{seat}
{
    m_toplevelUnmap
    .connect(toplevel->events.unmap)
    .set([this](auto)
    {
        m_seat.setInput<DefaultInput>(m_seat);
    });
}

PointerMove::~PointerMove() = default;

void PointerMove::onEnable()
{
    wlr_seat_pointer_notify_clear_focus(m_seat.getHandle());
    m_seat.cursor.setXcursor("grabbing");
}

void PointerMove::onDisable()
{
    // no-op
}

void PointerMove::onPointerButton(wlr_pointer_button_event* event)
{
    if (m_seat.cursor.getPointerButtonCount() == 0)
    {
        // If there is no button being pressed
        // we back to default.
        m_seat.setInput<DefaultInput>(m_seat);
    }
}

void PointerMove::onPointerMotion(uint32_t timeMsec)
{
    // Move the grabbed toplevel to the new position.
    m_toplevel->moveTo((m_seat.cursor.getPosition() - m_delta).into<int32_t>());
}

}