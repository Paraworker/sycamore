#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/input/Seat.h"
#include "sycamore/utils/List.h"

NAMESPACE_SYCAMORE_BEGIN

class InputManager {
public:
    void add(InputDevice* device);

    void remove(InputDevice* device);

    const List& getDeviceList(wlr_input_device_type type) const { return m_deviceList[type]; }

    static void onNewInput(wlr_input_device* handle);

public:
    List m_deviceList[INPUT_DEVICE_TYPE_NUM];

public:
    static InputManager instance;

private:
    InputManager();
    ~InputManager();
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_MANAGER_H