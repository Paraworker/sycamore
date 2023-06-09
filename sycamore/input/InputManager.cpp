#include "sycamore/input/InputManager.h"
#include "sycamore/input/Keyboard.h"
#include "sycamore/input/Pointer.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

InputManager InputManager::instance{};

InputManager::InputManager() = default;

InputManager::~InputManager() = default;

void InputManager::onNewInput(wlr_input_device* handle) {
    switch (handle->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            Keyboard::onCreate(handle);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            Pointer::onCreate(handle);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
            break;
        case WLR_INPUT_DEVICE_SWITCH:
            break;
    }
}

void InputManager::add(InputDevice* device) {
    m_deviceList[device->type()].add(device->link);
    Core::instance.seat->updateCapabilities();
}

void InputManager::remove(InputDevice* device) {
    m_deviceList[device->type()].remove(device->link);
    Core::instance.seat->updateCapabilities();
}

NAMESPACE_SYCAMORE_END