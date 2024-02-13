#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

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
     * @brief Update seat capabilities
     */
    void updateCapabilities() const;

    /**
     * @brief Sync LEDs with other keyboards
     */
    void syncKeyboardLeds(const Keyboard& keyboard) const;

    /**
     * @brief Add a new input device
     */
    void addDevice(wlr_input_device* handle);

    /**
     * @brief Remove an input device
     */
    template<typename T>
    void removeDevice(T* device)
    {
        remove(device);
        updateCapabilities();
    }

private:
    void addKeyboard(wlr_input_device* handle);
    void addPointer(wlr_input_device* handle);

    void remove(Keyboard* keyboard);
    void remove(Pointer* pointer);

private:
    std::list<Keyboard> m_keyboards;
    std::list<Pointer>  m_pointers;
};

inline InputManager inputManager{};

}

#endif //SYCAMORE_INPUT_MANAGER_H