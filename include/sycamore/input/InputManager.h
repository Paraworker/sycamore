#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/List.h"

NAMESPACE_SYCAMORE_BEGIN

class InputManager {
public:
    const List& getDeviceList(wlr_input_device_type type) const { return m_deviceList[type]; }

    void onNewDevice(wlr_input_device* handle);

    template<typename T>
    requires std::is_base_of_v<InputDevice, T>
    void onDestroyDevice(T* device) {
        removeDevice(device);
        delete device;
    }

public:
    static InputManager instance;

private:
    InputManager();

    ~InputManager();

    void addDevice(InputDevice* device);
    void removeDevice(InputDevice* device);

    template<typename T>
    requires std::is_base_of_v<InputDevice, T>
    void newDevice(wlr_input_device* handle) {
        addDevice(new T{handle});
    }

private:
    List m_deviceList[INPUT_DEVICE_TYPE_NUM];
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_MANAGER_H