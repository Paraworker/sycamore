#include "sycamore/input/state/PointerResize.h"

#include "sycamore/input/state/Passthrough.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/Core.h"

namespace sycamore
{

PointerResize::PointerResize(Toplevel* toplevel, uint32_t edges)
    : m_toplevel{toplevel}
    , m_edges{edges}
    , m_grabGeo{toplevel->geometry()}
{
    auto toplevelPos = toplevel->position();
    m_grabGeo.x += toplevelPos.x;
    m_grabGeo.y += toplevelPos.y;

    Point<double> border
    {
        static_cast<double>(m_grabGeo.x + ((edges & WLR_EDGE_RIGHT) ? m_grabGeo.width : 0)),
        static_cast<double>(m_grabGeo.y + ((edges & WLR_EDGE_BOTTOM) ? m_grabGeo.height : 0)),
    };

    m_delta = core.cursor.position() - border;

    m_toplevelUnmap = [](auto)
    {
        inputManager.toState<Passthrough>();
    };
    m_toplevelUnmap.connect(toplevel->events.unmap);
}

PointerResize::~PointerResize() = default;

void PointerResize::onEnable()
{
    m_toplevel->setResizing(true);
    wlr_seat_pointer_notify_clear_focus(core.seat->handle());
    core.cursor.setXcursor(wlr_xcursor_get_resize_name(static_cast<wlr_edges>(m_edges)));
}

void PointerResize::onDisable()
{
    m_toplevel->setResizing(false);
}

void PointerResize::onPointerButton(wlr_pointer_button_event* event)
{
    if (core.seat->pointerButtonCount() == 0)
    {
        // If there is no button being pressed
        // we back to passthrough
        inputManager.toState<Passthrough>();
    }
}

void PointerResize::onPointerMotion(uint32_t timeMsec)
{
    /* Resizing the grabbed toplevel can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the toplevel
     * on one or two axes, but can also move the toplevel if you resize from the top
     * or left edges (or top-left corner).
     *
     * Note that I took some shortcuts here. In a more fleshed-out compositor,
     * you'd wait for the client to prepare a buffer at the new size, then
     * commit any movement that was prepared. */
    int newLeft   = m_grabGeo.x;
    int newRight  = m_grabGeo.x + m_grabGeo.width;
    int newTop    = m_grabGeo.y;
    int newBottom = m_grabGeo.y + m_grabGeo.height;

    auto border = core.cursor.position() - m_delta;

    if (m_edges & WLR_EDGE_TOP)
    {
        newTop = border.y;
        if (newTop >= newBottom)
        {
            newTop = newBottom - 1;
        }
    }
    else if (m_edges & WLR_EDGE_BOTTOM)
    {
        newBottom = border.y;
        if (newBottom <= newTop)
        {
            newBottom = newTop + 1;
        }
    }

    if (m_edges & WLR_EDGE_LEFT)
    {
        newLeft = border.x;
        if (newLeft >= newRight)
        {
            newLeft = newRight - 1;
        }
    }
    else if (m_edges & WLR_EDGE_RIGHT)
    {
        newRight = border.x;
        if (newRight <= newLeft)
        {
            newRight = newLeft + 1;
        }
    }

    auto toplevelGeo = m_toplevel->geometry();

    m_toplevel->moveTo({newLeft - toplevelGeo.x, newTop - toplevelGeo.y});
    m_toplevel->setSize(newRight - newLeft, newBottom - newTop);
}

bool PointerResize::isInteractive() const
{
    return true;
}

}