#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/View.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/input/seatInput/ImplicitGrab.h"
#include "sycamore/utils/time.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

static inline void dragIconsUpdatePosition() {
    wlr_scene_node* node;
    wl_list_for_each(node, &Core::instance.scene->dragIcons->children, link) {
        static_cast<DragIconElement*>(node->data)->getIcon()->updatePosition();
    }
}

void DefaultInput::onEnable() {
    DefaultInput::rebasePointer();
}

void DefaultInput::onDisable() {
    // no-op
}

void DefaultInput::onPointerButton(wlr_pointer_button_event* event) {
    wlr_seat_pointer_notify_button(m_seat.getHandle(), event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_RELEASED) {
        return;
    }

    Point<double> sCoords{};
    if (auto element = Scene::elementFromNode(Core::instance.scene->nodeAt(m_seat.getCursor().getPosition(), sCoords)); element) {
        // If pressed on a view, focus it.
        if (element->type() == SceneElement::VIEW) {
            ShellManager::instance.setFocus(static_cast<ViewElement*>(element)->getView());
        }
    }

    auto& state = m_seat.getHandle()->pointer_state;

    // Start an implicit grab if seat has a focused surface
    if (state.focused_surface) {
        m_seat.setInput(new ImplicitGrab(state.focused_surface, {state.sx, state.sy}, m_seat));
    }
}

void DefaultInput::onPointerMotion(uint32_t timeMsec) {
    m_seat.updatePointerFocus(timeMsec);
    dragIconsUpdatePosition();
}

void DefaultInput::onPointerAxis(wlr_pointer_axis_event* event) {
    wlr_seat_pointer_notify_axis(m_seat.getHandle(), event->time_msec, event->orientation,
                                 event->delta, event->delta_discrete, event->source);
}

void DefaultInput::onPointerSwipeBegin(wlr_pointer_swipe_begin_event* event) {
    wlr_pointer_gestures_v1_send_swipe_begin(Core::instance.gestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void DefaultInput::onPointerSwipeUpdate(wlr_pointer_swipe_update_event* event) {
    wlr_pointer_gestures_v1_send_swipe_update(Core::instance.gestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy);
}

void DefaultInput::onPointerSwipeEnd(wlr_pointer_swipe_end_event* event) {
    wlr_pointer_gestures_v1_send_swipe_end(Core::instance.gestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void DefaultInput::onPointerPinchBegin(wlr_pointer_pinch_begin_event* event) {
    wlr_pointer_gestures_v1_send_pinch_begin(Core::instance.gestures,
                                             m_seat.getHandle(),
                                             event->time_msec, event->fingers);
}

void DefaultInput::onPointerPinchUpdate(wlr_pointer_pinch_update_event* event) {
    wlr_pointer_gestures_v1_send_pinch_update(Core::instance.gestures,
                                              m_seat.getHandle(),
                                              event->time_msec, event->dx, event->dy,
                                              event->scale, event->rotation);
}

void DefaultInput::onPointerPinchEnd(wlr_pointer_pinch_end_event* event) {
    wlr_pointer_gestures_v1_send_pinch_end(Core::instance.gestures,
                                           m_seat.getHandle(),
                                           event->time_msec, event->cancelled);
}

void DefaultInput::onPointerHoldBegin(wlr_pointer_hold_begin_event* event) {
    wlr_pointer_gestures_v1_send_hold_begin(Core::instance.gestures,
                                            m_seat.getHandle(),
                                            event->time_msec, event->fingers);
}

void DefaultInput::onPointerHoldEnd(wlr_pointer_hold_end_event* event) {
    wlr_pointer_gestures_v1_send_hold_end(Core::instance.gestures,
                                          m_seat.getHandle(),
                                          event->time_msec, event->cancelled);
}

void DefaultInput::rebasePointer() {
    if (m_seat.getCursor().isEnabled()) {
        m_seat.updatePointerFocus(getTimeMsec());
    }
}

NAMESPACE_SYCAMORE_END