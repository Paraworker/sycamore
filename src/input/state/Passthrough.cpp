#include "sycamore/input/state/Passthrough.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/state/ImplicitGrab.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/Seat.h"
#include "sycamore/utils/time.h"
#include "sycamore/Core.h"

namespace sycamore
{

void Passthrough::onEnable()
{
    Passthrough::rebasePointer();
}

void Passthrough::onDisable()
{
    // no-op
}

void Passthrough::onPointerButton(wlr_pointer_button_event* event)
{
    wlr_seat_pointer_notify_button(core.seat->handle(), event->time_msec, event->button, event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED)
    {
        return;
    }

    Point<double> sCoords{};
    if (auto element = scene::elementFromNode(core.scene.shellAt(core.cursor.position(), sCoords)); element)
    {
        // If pressed on a toplevel, focus it
        if (element->kind == scene::Element::TOPLEVEL)
        {
            windowManager.focusToplevel(static_cast<ToplevelElement*>(element)->toplevel);
        }
    }

    // Start an implicit grab if seat has a focused surface
    if (auto& state = core.seat->handle()->pointer_state; state.focused_surface)
    {
        inputManager.toState<ImplicitGrab>(state.focused_surface, Point{state.sx, state.sy});
    }
}

void Passthrough::onPointerMotion(uint32_t timeMsec)
{
    core.seat->updatePointerFocus(timeMsec);
    core.seat->updateDragIcons();
}

void Passthrough::onPointerAxis(wlr_pointer_axis_event* event)
{
    wlr_seat_pointer_notify_axis(core.seat->handle(),
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source, event->relative_direction);
}

void Passthrough::onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_begin(core.pointerGestures,
                                             core.seat->handle(),
                                             event->time_msec, event->fingers);
}

void Passthrough::onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_update(core.pointerGestures,
                                              core.seat->handle(),
                                              event->time_msec, event->dx, event->dy);
}

void Passthrough::onPointerSwipeEnd(wlr_pointer_swipe_end_event* event)
{
    wlr_pointer_gestures_v1_send_swipe_end(core.pointerGestures,
                                           core.seat->handle(),
                                           event->time_msec, event->cancelled);
}

void Passthrough::onPointerPinchBegin(wlr_pointer_pinch_begin_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_begin(core.pointerGestures,
                                             core.seat->handle(),
                                             event->time_msec, event->fingers);
}

void Passthrough::onPointerPinchUpdate(wlr_pointer_pinch_update_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_update(core.pointerGestures,
                                              core.seat->handle(),
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

void Passthrough::onPointerPinchEnd(wlr_pointer_pinch_end_event* event)
{
    wlr_pointer_gestures_v1_send_pinch_end(core.pointerGestures,
                                           core.seat->handle(),
                                           event->time_msec, event->cancelled);
}

void Passthrough::onPointerHoldBegin(wlr_pointer_hold_begin_event* event)
{
    wlr_pointer_gestures_v1_send_hold_begin(core.pointerGestures,
                                            core.seat->handle(),
                                            event->time_msec, event->fingers);
}

void Passthrough::onPointerHoldEnd(wlr_pointer_hold_end_event* event)
{
    wlr_pointer_gestures_v1_send_hold_end(core.pointerGestures,
                                          core.seat->handle(),
                                          event->time_msec, event->cancelled);
}

void Passthrough::rebasePointer()
{
    core.seat->updatePointerFocus(getMonotonic());
}

bool Passthrough::isInteractive() const
{
    return false;
}

}