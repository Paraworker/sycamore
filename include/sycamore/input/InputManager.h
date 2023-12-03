#ifndef SYCAMORE_INPUT_MANAGER_H
#define SYCAMORE_INPUT_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/input/Keyboard.h"
#include "sycamore/input/Pointer.h"

#include <memory>
#include <list>

NAMESPACE_SYCAMORE_BEGIN

class InputManager
{
public:
    using UPtr = std::unique_ptr<InputManager>;

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
     * @brief Return seat capabilities according to current devices
     */
    uint32_t capabilities() const;

    template<typename Func>
    void forEachKeyboard(Func&& func)
    {
        for (auto& keyboard : m_keyboards)
        {
            func(keyboard);
        }
    }

    template<typename Func>
    void forEachPointer(Func&& func)
    {
        for (auto& pointer : m_pointers)
        {
            func(pointer);
        }
    }

    void onNewDevice(wlr_input_device* handle);
    void onDestroyDevice(Keyboard* keyboard);
    void onDestroyDevice(Pointer* pointer);

    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager& operator=(InputManager&&) = delete;

private:
    void newKeyboard(wlr_input_device* handle);
    void newPointer(wlr_input_device* handle);

private:
    std::list<Keyboard> m_keyboards;
    std::list<Pointer>  m_pointers;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_INPUT_MANAGER_H