#include "sycamore/input/libinput.h"

#include <spdlog/spdlog.h>

static bool setAccelSpeed(libinput_device* device, double speed)
{
    if (!libinput_device_config_accel_is_available(device) ||
        libinput_device_config_accel_get_speed(device) == speed)
    {
        return false;
    }

    spdlog::debug("accel_set_speed({})", speed);
    libinput_device_config_accel_set_speed(device, speed);
    return true;
}

static bool setAccelProfile(libinput_device* device, libinput_config_accel_profile profile)
{
    if (!libinput_device_config_accel_is_available(device) ||
        libinput_device_config_accel_get_profile(device) == profile)
    {
        return false;
    }

    spdlog::debug("accel_set_profile({})", static_cast<int32_t>(profile));
    libinput_device_config_accel_set_profile(device, profile);
    return true;
}

static bool setTap(libinput_device* device, libinput_config_tap_state tap)
{
    if (libinput_device_config_tap_get_finger_count(device) <= 0 ||
        libinput_device_config_tap_get_enabled(device) == tap)
    {
        return false;
    }

    spdlog::debug("tap_set_enabled({})", static_cast<int32_t>(tap));
    libinput_device_config_tap_set_enabled(device, tap);
    return true;
}

static bool setNaturalScroll(libinput_device* device, bool enable)
{
    if (!libinput_device_config_scroll_has_natural_scroll(device) ||
        libinput_device_config_scroll_get_natural_scroll_enabled(device) == enable)
    {
        return false;
    }

    spdlog::debug("scroll_set_natural_scroll({})", enable);
    libinput_device_config_scroll_set_natural_scroll_enabled(device, enable);
    return true;
}

static bool deviceIsTouchpad(wlr_input_device* device)
{
    if (device->type != WLR_INPUT_DEVICE_POINTER ||
        !wlr_input_device_is_libinput(device))
    {
        return false;
    }

    auto handle = wlr_libinput_get_device_handle(device);
    return libinput_device_config_tap_get_finger_count(handle) > 0;
}

void touchpadSetNaturalScroll(wlr_input_device* device)
{
    if (!deviceIsTouchpad(device))
    {
        return;
    }

    auto handle = wlr_libinput_get_device_handle(device);
    setNaturalScroll(handle, true);
}

void touchpadSetTapToClick(wlr_input_device* device)
{
    if (!deviceIsTouchpad(device))
    {
        return;
    }

    auto handle = wlr_libinput_get_device_handle(device);
    setTap(handle, LIBINPUT_CONFIG_TAP_ENABLED);
}

void touchpadSetAccelSpeed(wlr_input_device* device, double speed)
{
    if (!deviceIsTouchpad(device))
    {
        return;
    }

    auto handle = wlr_libinput_get_device_handle(device);
    setAccelSpeed(handle, speed);
}