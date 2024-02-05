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
     * @brief Handle a new input device
     */
    void newDevice(wlr_input_device* handle);

    /**
     * @brief Destroy an input device
     */
    template<typename T>
    void destroyDevice(T* device)
    {
        destroy(device);
        updateCapabilities();
    }

private:
    void newKeyboard(wlr_input_device* handle);
    void newPointer(wlr_input_device* handle);

    void destroy(Keyboard* keyboard);
    void destroy(Pointer* pointer);

private:
    std::list<Keyboard> m_keyboards;
    std::list<Pointer>  m_pointers;
};

inline InputManager inputManager{};

}

#endif //SYCAMORE_INPUT_MANAGER_H