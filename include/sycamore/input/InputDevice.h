#ifndef SYCAMORE_INPUT_DEVICE_H
#define SYCAMORE_INPUT_DEVICE_H

#include "sycamore/defines.h"
#include "sycamore/wlroots.h"

#include <list>

NAMESPACE_SYCAMORE_BEGIN

template<typename ConcreteType>
class InputDevice
{
public:
    using Iter = std::list<ConcreteType>::iterator;

public:
    auto type() const
    {
        return m_deviceHandle->type;
    }

    auto name() const
    {
        return m_deviceHandle->name;
    }

    void iter(const Iter& itr)
    {
        m_iter = itr;
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

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_DEVICE_H