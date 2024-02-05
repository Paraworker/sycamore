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
        return m_baseHandle->type;
    }

    /**
     * @brief Get device name
     */
    auto name() const
    {
        return m_baseHandle->name;
    }

    /**
     * @brief Get wlr_input_device
     */
    auto getBaseHandle() const
    {
        return m_baseHandle;
    }

public:
    std::list<ConcreteType>::iterator iter;

protected:
    /**
     * @brief Constructor
     */
    explicit InputDevice(wlr_input_device* handle)
        : m_baseHandle{handle}
    {}

    /**
     * @brief Destructor
     */
    ~InputDevice() = default;

protected:
    wlr_input_device* m_baseHandle;
};

}

#endif //SYCAMORE_INPUT_DEVICE_H