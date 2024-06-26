#include "sycamore/input/InputManager.h"

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/desktop/WindowManager.h"
#include "sycamore/input/state/Passthrough.h"
#include "sycamore/input/Seat.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace sycamore
{

InputManager::InputManager() : state{std::make_unique<Passthrough>()}
{}

InputManager::~InputManager() = default;

void InputManager::updateCapabilities() const
{
    uint32_t caps{0};

    if (!m_pointers.empty()  /* || !m_tablets.empty() */)
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
        case WLR_INPUT_DEVICE_TABLET:
        case WLR_INPUT_DEVICE_TABLET_PAD:
        case WLR_INPUT_DEVICE_SWITCH:
            break;
        default:
            throw std::runtime_error{"unreachable!"};
    }

    updateCapabilities();
}

void InputManager::addKeyboard(wlr_input_device* handle)
{
    spdlog::info("New Keyboard: {}", handle->name);

    auto keyboard = m_keyboards.emplace(m_keyboards.end(), handle);

    keyboard->iter = keyboard;
    keyboard->apply();
}

void InputManager::addPointer(wlr_input_device* handle)
{
    spdlog::info("New Pointer: {}", handle->name);

    auto pointer = m_pointers.emplace(m_pointers.end(), handle);

    pointer->iter = pointer;
    pointer->apply();
}

void InputManager::remove(Keyboard* keyboard)
{
    spdlog::info("Remove Keyboard: {}", keyboard->name());
    m_keyboards.erase(keyboard->iter);
}

void InputManager::remove(Pointer* pointer)
{
    spdlog::info("Remove Pointer: {}", pointer->name());
    m_pointers.erase(pointer->iter);
}

bool InputManager::interactiveEnterCheck(const Toplevel& toplevel) const
{
    if (state->isInteractive())
    {
        return false;
    }

    if (toplevel.isPinned())
    {
        return false;
    }

    // Deny pointerMove/pointerResize for unfocused toplevel
    // or there is no focused toplevel.
    if (windowManager.focusState().toplevel != &toplevel)
    {
        return false;
    }

    return true;
}

}