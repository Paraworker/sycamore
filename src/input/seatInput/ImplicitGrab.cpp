#include "sycamore/input/seatInput/ImplicitGrab.h"

#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/Core.h"

namespace sycamore
{

ImplicitGrab::ImplicitGrab(wlr_surface* surface, const Point<double>& sCoords, Seat& seat)
    : m_surface{surface}
    , m_delta{sCoords - seat.cursor.getPosition()}
    , m_seat{seat}
{
    m_surfaceUnmap
    .connect(m_surface->events.unmap)
    .set([this](auto)
    {
        m_seat.setInput<DefaultInput>(m_seat);
    });
}

ImplicitGrab::~ImplicitGrab() = default;

void ImplicitGrab::onEnable()
{
    // no-op
}

void ImplicitGrab::onDisable()
{
    // no-op
}

void ImplicitGrab::onPointerButton(wlr_pointer_button_event* event)
{
    wlr_seat_pointer_notify_button(m_seat.getHandle(), event->time_msec,
                                   event->button, event->state);

    if (m_seat.cursor.getPointerButtonCount() == 0)
    {
        m_seat.setInput<DefaultInput>(m_seat);
    }
}

void ImplicitGrab::onPointerMotion(uint32_t timeMsec)
{
    auto relative = m_delta + m_seat.cursor.getPosition();
    wlr_seat_pointer_notify_motion(m_seat.getHandle(), timeMsec, relative.x, relative.y);
}

void ImplicitGrab::onPointerAxis(wlr_pointer_axis_event* event)
{
    wlr_seat_pointer_notify_axis(m_seat.getHandle(), event->time_msec, event->orientation,
                                 event->delta, event->delta_discrete, event->source);
}

void ImplicitGrab::onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_begin(Core::instance.gestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_update(Core::instance.gestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy);
}

void ImplicitGrab::onPointerSwipeEnd(wlr_pointer_swipe_end_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_end(Core::instance.gestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void ImplicitGrab::onPointerPinchBegin(wlr_pointer_pinch_begin_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_begin(Core::instance.gestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerPinchUpdate(wlr_pointer_pinch_update_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_update(Core::instance.gestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

void ImplicitGrab::onPointerPinchEnd(wlr_pointer_pinch_end_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_end(Core::instance.gestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void ImplicitGrab::onPointerHoldBegin(wlr_pointer_hold_begin_event* event)
{
    wlr_pointer_gestures_v1_send_hold_begin(Core::instance.gestures,
                                            m_seat.getHandle(),
                                            event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerHoldEnd(wlr_pointer_hold_end_event* event)
{
    wlr_pointer_gestures_v1_send_hold_end(Core::instance.gestures,
                                          m_seat.getHandle(),
                                          event->time_msec, event->cancelled);
}

}