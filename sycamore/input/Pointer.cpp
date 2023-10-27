#include "sycamore/input/Pointer.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/libinput.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Pointer::Pointer(wlr_input_device* deviceHandle)
    : InputDevice(deviceHandle), m_pointerHandle(wlr_pointer_from_input_device(deviceHandle)) {
    spdlog::info("New Pointer: {}", deviceHandle->name);

    Core::instance.seat->getCursor().attachDevice(deviceHandle);

    m_destroy.set(&deviceHandle->events.destroy, [this](void*) {
        InputManager::instance.onDestroyDevice(this);
    });

    applyConfig();
}

Pointer::~Pointer() {
    Core::instance.seat->getCursor().detachDevice(m_deviceHandle);
}

void Pointer::applyConfig() {
    // TODO: configurable
    touchpadSetTapToClick(m_deviceHandle);
    touchpadSetNaturalScroll(m_deviceHandle);
    touchpadSetAccelSpeed(m_deviceHandle, 0.3);
}

NAMESPACE_SYCAMORE_END