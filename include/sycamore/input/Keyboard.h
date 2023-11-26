#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Keyboard final : public InputDevice<Keyboard>
{
public:
    /**
     * @brief Constructor
     */
    explicit Keyboard(wlr_input_device* deviceHandle);

    /**
     * @brief Destructor
     */
    ~Keyboard();

    /**
     * @brief Apply config
     */
    void apply();

    /**
     * @brief Sync LEDs with other keyboards
     */
    void syncLeds();

    uint32_t getModifiers() const
    {
        return wlr_keyboard_get_modifiers(m_keyboardHandle);
    }

    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) = delete;

private:
    wlr_keyboard* m_keyboardHandle;

private:
    Listener m_modifiers;
    Listener m_key;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_KEYBOARD_H