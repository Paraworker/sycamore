#include "sycamore/input/InputManager.h"

#include "sycamore/Core.h"
#include <spdlog/spdlog.h>

namespace sycamore
{

void InputManager::updateCapabilities() const
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

    core.seat->setCapabilities(caps);
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

void InputManager::newDevice(wlr_input_device* handle)
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

    updateCapabilities();
}

void InputManager::newKeyboard(wlr_input_device* handle)
{
    spdlog::info("New Keyboard: {}", handle->name);

    auto keyboard = m_keyboards.emplace(m_keyboards.end(), handle);

    keyboard->iter = keyboard;
    keyboard->apply();
}

void InputManager::newPointer(wlr_input_device* handle)
{
    spdlog::info("New Pointer: {}", handle->name);

    auto pointer = m_pointers.emplace(m_pointers.end(), handle);

    pointer->iter = pointer;
    pointer->apply();

    core.seat->cursor.attachDevice(handle);
}

void InputManager::destroy(Keyboard* keyboard)
{
    spdlog::info("Destroy Keyboard: {}", keyboard->name());
    m_keyboards.erase(keyboard->iter);
}

void InputManager::destroy(Pointer* pointer)
{
    spdlog::info("Destroy Pointer: {}", pointer->name());
    core.seat->cursor.detachDevice(pointer->getBaseHandle());
    m_pointers.erase(pointer->iter);
}

}