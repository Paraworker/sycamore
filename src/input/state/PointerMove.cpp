#include "sycamore/input/state/PointerMove.h"

#include "sycamore/input/state/Passthrough.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/Core.h"

namespace sycamore
{

PointerMove::PointerMove(Toplevel* toplevel)
    : m_toplevel{toplevel}
    , m_delta{core.cursor.position() - static_cast<Point<double>>(toplevel->position())}
{
    m_toplevelUnmap = [](auto)
    {
        inputManager.toState<Passthrough>();
    };
    m_toplevelUnmap.connect(toplevel->events.unmap);
}

PointerMove::~PointerMove() = default;

void PointerMove::onEnable()
{
    wlr_seat_pointer_notify_clear_focus(core.seat->handle());
    core.cursor.setXcursor("grabbing");
}

void PointerMove::onDisable()
{
    // no-op
}

void PointerMove::onPointerButton(wlr_pointer_button_event* event)
{
    if (core.seat->pointerButtonCount() == 0)
    {
        // If there is no button being pressed
        // we back to passthrough
        inputManager.toState<Passthrough>();
    }
}

void PointerMove::onPointerMotion(uint32_t timeMsec)
{
    // Move the grabbed toplevel to the new position.
    m_toplevel->moveTo(static_cast<Point<int32_t>>(core.cursor.position() - m_delta));
}

bool PointerMove::isInteractive() const
{
    return true;
}

}