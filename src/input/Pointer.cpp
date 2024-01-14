#include "sycamore/input/Pointer.h"

#include "sycamore/input/InputManager.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Pointer::Pointer(wlr_input_device* deviceHandle)
    : InputDevice{deviceHandle}
    , m_pointerHandle{wlr_pointer_from_input_device(deviceHandle)}
{
    spdlog::info("New Pointer: {}", deviceHandle->name);

    core.seat->cursor.attachDevice(deviceHandle);

    m_destroy.notify([this](auto)
    {
        InputManager::instance.onDestroyDevice(this);
    });
    m_destroy.connect(deviceHandle->events.destroy);

    apply();
}

Pointer::~Pointer()
{
    core.seat->cursor.detachDevice(m_deviceHandle);
}

bool Pointer::isTouchpad() const
{
    if (!wlr_input_device_is_libinput(m_deviceHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_deviceHandle);
    return libinput_device_config_tap_get_finger_count(handle) > 0;
}

bool Pointer::setNaturalScroll(bool enable)
{
    if (!wlr_input_device_is_libinput(m_deviceHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_deviceHandle);

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
    if (!wlr_input_device_is_libinput(m_deviceHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_deviceHandle);

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
    if (!wlr_input_device_is_libinput(m_deviceHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_deviceHandle);

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
    if (!wlr_input_device_is_libinput(m_deviceHandle))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(m_deviceHandle);

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