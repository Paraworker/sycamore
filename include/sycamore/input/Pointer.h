#ifndef SYCAMORE_POINTER_H
#define SYCAMORE_POINTER_H

#include "sycamore/defines.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Pointer final : public InputDevice
{
public:
    /**
     * @brief Constructor
     */
    explicit Pointer(wlr_input_device* deviceHandle);

    /**
     * @brief Destructor
     */
    ~Pointer();

    /**
     * @brief Whether the pointer device is a touchpad
     */
    bool isTouchpad() const;

    /**
     * @brief Set natural scroll
     * @note Only available for touchpad
     */
    bool setNaturalScroll(bool enable);

    /**
     * @brief Set tap to click
     * @note Only available for touchpad
     */
    bool setTapToClick(libinput_config_tap_state state);

    /**
     * @brief Set accel speed
     */
    bool setAccelSpeed(double speed);

    /**
     * @brief Set accel profile
     */
    bool setAccelProfile(libinput_config_accel_profile profile);

    /**
     * @brief Apply config
     */
    void apply();

    Pointer(const Pointer&) = delete;
    Pointer(Pointer&&) = delete;
    Pointer& operator=(const Pointer&) = delete;
    Pointer& operator=(Pointer&&) = delete;

private:
    wlr_pointer* m_pointerHandle;

private:
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POINTER_H