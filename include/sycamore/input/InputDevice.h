#ifndef SYCAMORE_INPUT_DEVICE_H
#define SYCAMORE_INPUT_DEVICE_H

#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

template<typename ConcreteType>
class InputDevice
{
public:
    /**
     * @brief Get wlr_input_device_type
     */
    auto type() const
    {
        return m_deviceHandle->type;
    }

    /**
     * @brief Get device name
     */
    auto name() const
    {
        return m_deviceHandle->name;
    }

public:
    std::list<ConcreteType>::iterator iter;

protected:
    /**
     * @brief Constructor
     */
    explicit InputDevice(wlr_input_device* handle)
        : m_deviceHandle{handle}
    {}

    /**
     * @brief Destructor
     */
    ~InputDevice() = default;

protected:
    wlr_input_device* m_deviceHandle;
};

}

#endif //SYCAMORE_INPUT_DEVICE_H