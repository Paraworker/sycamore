#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/List.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

class InputManager
{
public:
    /**
     * @brief Get the list of a certain type device
     */
    const List& getDeviceList(wlr_input_device_type type) const { return m_deviceList[type]; }

    void onNewDevice(wlr_input_device* handle);

    template<typename T>
    requires std::is_base_of_v<InputDevice, T>
    void onDestroyDevice(T* device)
    {
        m_deviceList[device->type()].remove(device->link);
        delete device;

        Core::instance.seat->updateCapabilities();
    }

public:
    static InputManager instance;

private:
    /**
     * @brief Constructor
     */
    InputManager() = default;

    /**
     * @brief Destructor
     */
    ~InputManager() = default;

    template<typename T>
    requires std::is_base_of_v<InputDevice, T>
    void newDevice(wlr_input_device* handle)
    {
        auto device = new T{handle};
        m_deviceList[device->type()].add(device->link);

        Core::instance.seat->updateCapabilities();
    }

private:
    List m_deviceList[INPUT_DEVICE_TYPE_NUM];
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_MANAGER_H