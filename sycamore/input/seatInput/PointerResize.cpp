#include "sycamore/input/InputManager.h"
#include "sycamore/input/seatInput/PointerResize.h"
#include "sycamore/input/seatInput/DefaultInput.h"

NAMESPACE_SYCAMORE_BEGIN

PointerResize::PointerResize(View* view, uint32_t edges, Seat& seat)
    : m_view(view)
    , m_edges(edges)
    , m_grabGeo(view->getGeometry())
    , m_seat(seat)
{
    auto viewPos = view->getPosition();
    m_grabGeo.x += viewPos.x;
    m_grabGeo.y += viewPos.y;

    Point<int> border
    {
        m_grabGeo.x + ((edges & WLR_EDGE_RIGHT) ? m_grabGeo.width : 0),
        m_grabGeo.y + ((edges & WLR_EDGE_BOTTOM) ? m_grabGeo.height : 0)
    };

    m_delta = m_seat.getCursor().getPosition() - border.into<double>();

    m_viewUnmap
    .connect(m_view->events.unmap)
    .set([this](void*) { m_seat.setInput(new DefaultInput{m_seat}); });
}

PointerResize::~PointerResize() = default;

void PointerResize::onEnable()
{
    m_view->setResizing(true);
    wlr_seat_pointer_notify_clear_focus(m_seat.getHandle());
    m_seat.getCursor().setXcursor(wlr_xcursor_get_resize_name(static_cast<wlr_edges>(m_edges)));
}

void PointerResize::onDisable()
{
    m_view->setResizing(false);
}

void PointerResize::onPointerButton(wlr_pointer_button_event* event)
{
    if (m_seat.getCursor().getPointerButtonCount() == 0)
    {
        // If there is no button being pressed
        // we back to default.
        m_seat.setInput(new DefaultInput{m_seat});
    }
}

void PointerResize::onPointerMotion(uint32_t timeMsec)
{
    /* Resizing the grabbed view can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the view
     * on one or two axes, but can also move the view if you resize from the top
     * or left edges (or top-left corner).
     *
     * Note that I took some shortcuts here. In a more fleshed-out compositor,
     * you'd wait for the client to prepare a buffer at the new size, then
     * commit any movement that was prepared. */
    int newLeft   = m_grabGeo.x;
    int newRight  = m_grabGeo.x + m_grabGeo.width;
    int newTop    = m_grabGeo.y;
    int newBottom = m_grabGeo.y + m_grabGeo.height;

    auto border = m_seat.getCursor().getPosition() - m_delta;

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

    auto viewGeoBox = m_view->getGeometry();

    m_view->moveTo({newLeft - viewGeoBox.x, newTop - viewGeoBox.y});
    m_view->setSize(newRight - newLeft, newBottom - newTop);
}

NAMESPACE_SYCAMORE_END