#include "sycamore/input/InputManager.h"

#include "sycamore/Core.h"

namespace sycamore
{

uint32_t InputManager::capabilities() const
{
    uint32_t caps{0};

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

void InputManager::syncKeyboardLeds(const Keyboard& keyboard) const
{
    const auto leds = keyboard.ledsState();

    for (auto& item : m_keyboards)
    {
        if (item != keyboard)
        {
            item.updateLeds(leds);
        }
    }
}

void InputManager::addDevice(wlr_input_device* handle)
{
    switch (handle->type)
    {
        case WLR_INPUT_DEVICE_KEYBOARD:
            addKeyboard(handle);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            addPointer(handle);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
        case WLR_INPUT_DEVICE_TABLET_TOOL:
        case WLR_INPUT_DEVICE_TABLET_PAD:
        case WLR_INPUT_DEVICE_SWITCH:
        default:
            break;
    }
}

void InputManager::removeDevice(Keyboard* keyboard)
{
    m_keyboards.erase(keyboard->iter);
    core.seat->setCapabilities(capabilities());
}

void InputManager::removeDevice(Pointer* pointer)
{
    m_pointers.erase(pointer->iter);
    core.seat->setCapabilities(capabilities());
}

void InputManager::addKeyboard(wlr_input_device* handle)
{
    auto keyboard = m_keyboards.emplace(m_keyboards.end(), handle);
    keyboard->iter = keyboard;

    core.seat->setCapabilities(capabilities());
}

void InputManager::addPointer(wlr_input_device* handle)
{
    auto pointer = m_pointers.emplace(m_pointers.end(), handle);
    pointer->iter = pointer;

    core.seat->setCapabilities(capabilities());
}

}