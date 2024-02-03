#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/input/InputDevice.h"
#include "sycamore/input/Keyboard.h"
#include "sycamore/input/Pointer.h"

#include <list>

namespace sycamore
{

class InputManager
{
public:
    /**
     * @brief Constructor
     */
    InputManager() = default;

    /**
     * @brief Destructor
     */
    ~InputManager() = default;

    /**
     * @brief Get seat capabilities
     */
    uint32_t capabilities() const;

    /**
     * @brief Sync LEDs with other keyboards
     */
    void syncKeyboardLeds(const Keyboard& keyboard) const;

    void addDevice(wlr_input_device* handle);
    void removeDevice(Keyboard* keyboard);
    void removeDevice(Pointer* pointer);

private:
    void addKeyboard(wlr_input_device* handle);
    void addPointer(wlr_input_device* handle);

private:
    std::list<Keyboard> m_keyboards;
    std::list<Pointer>  m_pointers;
};

inline InputManager inputManager{};

}

#endif //SYCAMORE_INPUT_MANAGER_H