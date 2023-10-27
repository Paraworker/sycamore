#include "sycamore/input/InputManager.h"
#include "sycamore/input/Keyboard.h"
#include "sycamore/input/Pointer.h"

NAMESPACE_SYCAMORE_BEGIN

InputManager InputManager::instance{};

void InputManager::onNewDevice(wlr_input_device* handle) {
    switch (handle->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            newDevice<Keyboard>(handle);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            newDevice<Pointer>(handle);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
        case WLR_INPUT_DEVICE_TABLET_TOOL:
        case WLR_INPUT_DEVICE_TABLET_PAD:
        case WLR_INPUT_DEVICE_SWITCH:
        default:
            break;
    }
}

NAMESPACE_SYCAMORE_END