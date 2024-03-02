#include "sycamore/input/Pointer.h"

#include "sycamore/input/InputManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Pointer::Pointer(wlr_input_device* baseHandle)
    : InputDevice{baseHandle}
    , m_pointerHandle{wlr_pointer_from_input_device(baseHandle)}
{
    m_motion = [](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_event*>(data);

        core.cursor.move({event->delta_x, event->delta_y}, &event->pointer->base);

        core.seat->enablePointer();
        core.seat->input->onPointerMotion(event->time_msec);
    };
    m_motion.connect(m_pointerHandle->events.motion);

    m_motionAbsolute = [](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);

        core.cursor.warpAbsolute({event->x, event->y}, &event->pointer->base);

        core.seat->enablePointer();
        core.seat->input->onPointerMotion(event->time_msec);
    };
    m_motionAbsolute.connect(m_pointerHandle->events.motion_absolute);

    m_button = [](void* data)
    {
        auto event = static_cast<wlr_pointer_button_event*>(data);

        core.seat->updatePointerButtonCount(event->state);

        core.seat->enablePointer();
        core.seat->input->onPointerButton(event);
    };
    m_button.connect(m_pointerHandle->events.button);

    m_axis = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerAxis(static_cast<wlr_pointer_axis_event*>(data));
    };
    m_axis.connect(m_pointerHandle->events.axis);

    m_frame = [](auto)
    {
        core.seat->enablePointer();
        wlr_seat_pointer_notify_frame(core.seat->handle());
    };
    m_frame.connect(m_pointerHandle->events.frame);

    m_swipeBegin = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerSwipeBegin(static_cast<wlr_pointer_swipe_begin_event*>(data));
    };
    m_swipeBegin.connect(m_pointerHandle->events.swipe_begin);

    m_swipeUpdate = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerSwipeUpdate(static_cast<wlr_pointer_swipe_update_event*>(data));
    };
    m_swipeUpdate.connect(m_pointerHandle->events.swipe_update);

    m_swipeEnd = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerSwipeEnd(static_cast<wlr_pointer_swipe_end_event*>(data));
    };
    m_swipeEnd.connect(m_pointerHandle->events.swipe_end);

    m_pinchBegin = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerPinchBegin(static_cast<wlr_pointer_pinch_begin_event*>(data));
    };
    m_pinchBegin.connect(m_pointerHandle->events.pinch_begin);

    m_pinchUpdate = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerPinchUpdate(static_cast<wlr_pointer_pinch_update_event*>(data));
    };
    m_pinchUpdate.connect(m_pointerHandle->events.pinch_update);

    m_pinchEnd = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerPinchEnd(static_cast<wlr_pointer_pinch_end_event*>(data));
    };
    m_pinchEnd.connect(m_pointerHandle->events.pinch_end);

    m_holdBegin = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerHoldBegin(static_cast<wlr_pointer_hold_begin_event*>(data));
    };
    m_holdBegin.connect(m_pointerHandle->events.hold_begin);

    m_holdEnd = [](void* data)
    {
        core.seat->enablePointer();
        core.seat->input->onPointerHoldEnd(static_cast<wlr_pointer_hold_end_event*>(data));
    };
    m_holdEnd.connect(m_pointerHandle->events.hold_end);

    m_destroy = [this](auto)
    {
        inputManager.removeDevice(this);
    };
    m_destroy.connect(baseHandle->events.destroy);
}

Pointer::~Pointer() = default;

bool Pointer::isTouchpad() const
{
    if (!wlr_input_device_is_libinput(m_baseHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_baseHandle);
    return libinput_device_config_tap_get_finger_count(handle) > 0;
}

bool Pointer::setNaturalScroll(bool enable)
{
    if (!wlr_input_device_is_libinput(m_baseHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_baseHandle);

    if (!libinput_device_config_scroll_has_natural_scroll(handle) ||
        libinput_device_config_scroll_get_natural_scroll_enabled(handle) == enable)
    {
        return false;
    }

    spdlog::debug("scroll_set_natural_scroll({})", enable);
    libinput_device_config_scroll_set_natural_scroll_enabled(handle, enable);
    return true;
}

bool Pointer::setTapToClick(libinput_config_tap_state state)
{
    if (!wlr_input_device_is_libinput(m_baseHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_baseHandle);

    if (libinput_device_config_tap_get_finger_count(handle) <= 0 ||
        libinput_device_config_tap_get_enabled(handle) == state)
    {
        return false;
    }

    spdlog::debug("tap_set_enabled({})", static_cast<int32_t>(state));
    libinput_device_config_tap_set_enabled(handle, state);
    return true;
}

bool Pointer::setAccelSpeed(double speed)
{
    if (!wlr_input_device_is_libinput(m_baseHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_baseHandle);

    if (!libinput_device_config_accel_is_available(handle) ||
        libinput_device_config_accel_get_speed(handle) == speed)
    {
        return false;
    }

    spdlog::debug("accel_set_speed({})", speed);
    libinput_device_config_accel_set_speed(handle, speed);
    return true;
}

bool Pointer::setAccelProfile(libinput_config_accel_profile profile)
{
    if (!wlr_input_device_is_libinput(m_baseHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_baseHandle);

    if (!libinput_device_config_accel_is_available(handle) ||
        libinput_device_config_accel_get_profile(handle) == profile)
    {
        return false;
    }

    spdlog::debug("accel_set_profile({})", static_cast<int32_t>(profile));
    libinput_device_config_accel_set_profile(handle, profile);
    return true;
}

void Pointer::apply()
{
    // TODO: configurable
    if (isTouchpad())
    {
        setTapToClick(LIBINPUT_CONFIG_TAP_ENABLED);
        setNaturalScroll(true);
        setAccelSpeed(0.3);
    }
}

}