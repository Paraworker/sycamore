#include "sycamore/input/InputManager.h"
#include "sycamore/input/seatInput/PointerMove.h"
#include "sycamore/input/seatInput/DefaultInput.h"

NAMESPACE_SYCAMORE_BEGIN

PointerMove::PointerMove(View* view, Seat& seat)
    : m_view(view)
    , m_delta(seat.getCursor().getPosition() - view->getPosition().into<double>())
    , m_seat(seat) {
    m_viewUnmap.set(&m_view->events.unmap, [this](void*) {
        m_seat.setInput(new DefaultInput(m_seat));
    });
}

PointerMove::~PointerMove() = default;

void PointerMove::onEnable() {
    wlr_seat_pointer_notify_clear_focus(m_seat.getHandle());
    m_seat.getCursor().setXcursor("grabbing");
}

void PointerMove::onDisable() {
    // no-op
}

void PointerMove::onPointerButton(wlr_pointer_button_event* event) {
    if (m_seat.getCursor().getPointerButtonCount() == 0) {
        // If there is no button being pressed
        // we back to default.
        m_seat.setInput(new DefaultInput(m_seat));
    }
}

void PointerMove::onPointerMotion(uint32_t timeMsec) {
    // Move the grabbed view to the new position.
    m_view->moveTo((m_seat.getCursor().getPosition() - m_delta).into<int32_t>());
}

NAMESPACE_SYCAMORE_END