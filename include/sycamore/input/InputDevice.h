#ifndef SYCAMORE_INPUT_DEVICE_H
#define SYCAMORE_INPUT_DEVICE_H

#include "sycamore/defines.h"
#include "sycamore/wlroots.h"

NAMESPACE_SYCAMORE_BEGIN

class InputDevice
{
public:
    auto type() const { return m_deviceHandle->type; }

    auto name() const { return m_deviceHandle->name; }

public:
    wl_list link{}; // InputManager::deviceList

protected:
    explicit InputDevice(wlr_input_device* handle)
        : m_deviceHandle{handle} {}

    ~InputDevice() = default;

protected:
    wlr_input_device* m_deviceHandle;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_DEVICE_H