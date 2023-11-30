#include "sycamore/input/InputManager.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

InputManager InputManager::instance{};

uint32_t InputManager::capabilities() const
{
    uint32_t caps = 0;

    if (!m_pointers.empty()  /* || !m_tabletTools.empty() */)
    {
        caps |= WL_SEAT_CAPABILITY_POINTER;
    }

    if (!m_keyboards.empty())
    {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    /*
    if (!m_touchs.empty())
    {
        caps |= WL_SEAT_CAPABILITY_TOUCH;
    }
     */

    return caps;
}

void InputManager::onNewDevice(wlr_input_device* handle)
{
    switch (handle->type)
    {
        case WLR_INPUT_DEVICE_KEYBOARD:
            newKeyboard(handle);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            newPointer(handle);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
        case WLR_INPUT_DEVICE_TABLET_TOOL:
        case WLR_INPUT_DEVICE_TABLET_PAD:
        case WLR_INPUT_DEVICE_SWITCH:
        default:
            break;
    }
}

void InputManager::onDestroyDevice(Keyboard* keyboard)
{
    m_keyboards.erase(keyboard->iter());
    Core::instance.seat->setCapabilities(capabilities());
}

void InputManager::onDestroyDevice(Pointer* pointer)
{
    m_pointers.erase(pointer->iter());
    Core::instance.seat->setCapabilities(capabilities());
}

void InputManager::newKeyboard(wlr_input_device* handle)
{
    auto iter = m_keyboards.emplace(m_keyboards.end(), handle);
    iter->iter(iter);

    Core::instance.seat->setCapabilities(capabilities());
}

void InputManager::newPointer(wlr_input_device* handle)
{
    auto iter = m_pointers.emplace(m_pointers.end(), handle);
    iter->iter(iter);

    Core::instance.seat->setCapabilities(capabilities());
}

NAMESPACE_SYCAMORE_END