#include "sycamore/input/Pointer.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/libinput.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Pointer* Pointer::create(wlr_input_device *deviceHandle) {
    spdlog::info("New Pointer: {}", deviceHandle->name);

    // Default config
    touchpadSetTapToClick(deviceHandle);
    touchpadSetNaturalScroll(deviceHandle);
    touchpadSetAccelSpeed(deviceHandle, 0.3);

    Core::instance.seat->getCursor().attachDevice(deviceHandle);

    auto pointer = new Pointer{deviceHandle, wlr_pointer_from_input_device(deviceHandle)};

    InputManager::instance.add(pointer);

    return pointer;
}

Pointer::Pointer(wlr_input_device* deviceHandle, wlr_pointer* pointerHandle)
    : InputDevice(deviceHandle), m_pointerHandle(pointerHandle) {
    m_destroy.set(&deviceHandle->events.destroy, [this](void*) {
        Core::instance.seat->getCursor().detachDevice(m_deviceHandle);
        InputManager::instance.remove(this);
        delete this;
    });
}

Pointer::~Pointer() = default;

NAMESPACE_SYCAMORE_END