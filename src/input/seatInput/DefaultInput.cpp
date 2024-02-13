#include "sycamore/input/seatInput/DefaultInput.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/seatInput/ImplicitGrab.h"
#include "sycamore/utils/time.h"
#include "sycamore/Core.h"

namespace sycamore
{

void DefaultInput::onEnable()
{
    DefaultInput::rebasePointer();
}

void DefaultInput::onDisable()
{
    // no-op
}

void DefaultInput::onPointerButton(wlr_pointer_button_event* event)
{
    wlr_seat_pointer_notify_button(m_seat.getHandle(), event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_RELEASED)
    {
        return;
    }

    Point<double> sCoords{};
    if (auto element = scene::elementFromNode(core.scene.shellAt(m_seat.cursor.position(), sCoords)); element)
    {
        // If pressed on a toplevel, focus it
        if (element->kind == scene::Element::TOPLEVEL)
        {
            windowManager.focusToplevel(static_cast<ToplevelElement*>(element)->toplevel);
        }
    }

    // Start an implicit grab if seat has a focused surface
    if (auto& state = m_seat.getHandle()->pointer_state; state.focused_surface)
    {
        m_seat.setInput<ImplicitGrab>(state.focused_surface, Point{state.sx, state.sy}, m_seat);
    }
}

void DefaultInput::onPointerMotion(uint32_t timeMsec)
{
    m_seat.updatePointerFocus(timeMsec);
    m_seat.updateDragIconsPosition();
}

void DefaultInput::onPointerAxis(wlr_pointer_axis_event* event)
{
    wlr_seat_pointer_notify_axis(m_seat.getHandle(),
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source, event->relative_direction);
}

void DefaultInput::onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_begin(core.pointerGestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void DefaultInput::onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_update(core.pointerGestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy);
}

void DefaultInput::onPointerSwipeEnd(wlr_pointer_swipe_end_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_end(core.pointerGestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void DefaultInput::onPointerPinchBegin(wlr_pointer_pinch_begin_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_begin(core.pointerGestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void DefaultInput::onPointerPinchUpdate(wlr_pointer_pinch_update_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_update(core.pointerGestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

void DefaultInput::onPointerPinchEnd(wlr_pointer_pinch_end_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_end(core.pointerGestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void DefaultInput::onPointerHoldBegin(wlr_pointer_hold_begin_event* event)
{
    wlr_pointer_gestures_v1_send_hold_begin(core.pointerGestures,
                                            m_seat.getHandle(),
                                            event->time_msec, event->fingers);
}

void DefaultInput::onPointerHoldEnd(wlr_pointer_hold_end_event* event)
{
    wlr_pointer_gestures_v1_send_hold_end(core.pointerGestures,
                                          m_seat.getHandle(),
                                          event->time_msec, event->cancelled);
}

void DefaultInput::rebasePointer()
{
    if (m_seat.cursor.isEnabled())
    {
        m_seat.updatePointerFocus(getMonotonic());
    }
}

}