#ifndef SYCAMORE_KEYBOARD_H
#define SYCAMORE_KEYBOARD_H

#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

namespace sycamore
{

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

    uint32_t getModifiers() const noexcept
    {
        return wlr_keyboard_get_modifiers(m_keyboardHandle);
    }

    /**
     * @brief Get LEDs state
     */
    uint32_t ledsState() const;

    /**
     * @brief Update LEDs state
     */
    void updateLeds(uint32_t leds) const
    {
        wlr_keyboard_led_update(m_keyboardHandle, leds);
    }

    bool operator==(const Keyboard& rhs) const
    {
        return m_keyboardHandle == rhs.m_keyboardHandle;
    }

    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) = delete;

private:
    wlr_keyboard* m_keyboardHandle;

    Listener      m_modifiers;
    Listener      m_key;
    Listener      m_destroy;
};

}

#endif //SYCAMORE_KEYBOARD_H