#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/input/Keyboard.h"
#include "sycamore/input/Pointer.h"
#include "sycamore/input/state/InputState.h"

#include <list>
#include <memory>

namespace sycamore
{

class Toplevel;

class InputManager
{
public:
    /**
     * @brief Constructor
     */
    InputManager();

    /**
     * @brief Destructor
     */
    ~InputManager();

    /**
     * @brief Switch to a new input state
     */
    template<typename T, typename... Args>
    void toState(Args&&... args)
    {
        state->onDisable();
        state.reset(new T{std::forward<Args>(args)...});
        state->onEnable();
    }

    /**
     * @brief Check whether an 'interactive' input state should begin
     */
    bool interactiveEnterCheck(const Toplevel& toplevel) const;

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

public:
    std::unique_ptr<InputState> state;

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