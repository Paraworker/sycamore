#include "sycamore/input/state/ImplicitGrab.h"

#include "sycamore/input/state/Passthrough.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/Core.h"

namespace sycamore
{

ImplicitGrab::ImplicitGrab(wlr_surface* surface, const Point<double>& sCoords)
    : m_surface{surface}
    , m_delta{sCoords - core.cursor.position()}
{
    m_surfaceUnmap = [](auto)
    {
        inputManager.toState<Passthrough>();
    };
    m_surfaceUnmap.connect(m_surface->events.unmap);
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
    wlr_seat_pointer_notify_button(core.seat->handle(),
                                   event->time_msec, event->button, event->state);

    if (core.seat->pointerButtonCount() == 0)
    {
        inputManager.toState<Passthrough>();
    }
}

void ImplicitGrab::onPointerMotion(uint32_t timeMsec)
{
    auto relative = m_delta + core.cursor.position();
    wlr_seat_pointer_notify_motion(core.seat->handle(), timeMsec, relative.x, relative.y);
}

void ImplicitGrab::onPointerAxis(wlr_pointer_axis_event* event)
{
    wlr_seat_pointer_notify_axis(core.seat->handle(),
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source, event->relative_direction);
}

void ImplicitGrab::onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_begin(core.pointerGestures,
                                             core.seat->handle(),
                                             event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_update(core.pointerGestures,
                                              core.seat->handle(),
                                              event->time_msec, event->dx, event->dy);
}

void ImplicitGrab::onPointerSwipeEnd(wlr_pointer_swipe_end_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_end(core.pointerGestures,
                                           core.seat->handle(),
                                           event->time_msec, event->cancelled);
}

void ImplicitGrab::onPointerPinchBegin(wlr_pointer_pinch_begin_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_begin(core.pointerGestures,
                                             core.seat->handle(),
                                             event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerPinchUpdate(wlr_pointer_pinch_update_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_update(core.pointerGestures,
                                              core.seat->handle(),
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

void ImplicitGrab::onPointerPinchEnd(wlr_pointer_pinch_end_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_end(core.pointerGestures,
                                           core.seat->handle(),
                                           event->time_msec, event->cancelled);
}

void ImplicitGrab::onPointerHoldBegin(wlr_pointer_hold_begin_event* event)
{
    wlr_pointer_gestures_v1_send_hold_begin(core.pointerGestures,
                                            core.seat->handle(),
                                            event->time_msec, event->fingers);
}

void ImplicitGrab::onPointerHoldEnd(wlr_pointer_hold_end_event* event)
{
    wlr_pointer_gestures_v1_send_hold_end(core.pointerGestures,
                                          core.seat->handle(),
                                          event->time_msec, event->cancelled);
}

bool ImplicitGrab::isInteractive() const
{
    return false;
}

}