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
    using Iter = std::list<ConcreteType>::iterator;

public:
    /**
     * @brief Get wlr_input_device_type
     */
    auto type() const noexcept
    {
        return m_deviceHandle->type;
    }

    /**
     * @brief Get device name
     */
    auto name() const noexcept
    {
        return m_deviceHandle->name;
    }

    void iter(const Iter& iter)
    {
        m_iter = iter;
    }

    Iter& iter()
    {
        return m_iter;
    }

protected:
    /**
     * @brief Constructor
     */
    explicit InputDevice(wlr_input_device* handle)
        : m_deviceHandle{handle} {}

    /**
     * @brief Destructor
     */
    ~InputDevice() = default;

protected:
    wlr_input_device* m_deviceHandle;
    Iter              m_iter;
};

}

#endif //SYCAMORE_INPUT_DEVICE_H