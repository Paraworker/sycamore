#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Keyboard final : public InputDevice {
public:
    /**
     * @brief Create Keyboard
     * @return nullptr on failure
     */
    static Keyboard* create(wlr_input_device* deviceHandle);

    uint32_t getModifiers() const {
        return wlr_keyboard_get_modifiers(m_keyboardHandle);
    }

    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) = delete;

private:
    Keyboard(wlr_input_device* deviceHandle, wlr_keyboard* keyboardHandle);

    ~Keyboard();

    /**
     * @brief Sync LEDs with other keyboards
     */
    void syncLeds();

private:
    wlr_keyboard* m_keyboardHandle;

private:
    Listener m_modifiers;
    Listener m_key;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_KEYBOARD_H