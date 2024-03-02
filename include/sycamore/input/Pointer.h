#ifndef SYCAMORE_POINTER_H
#define SYCAMORE_POINTER_H

#include "sycamore/input/InputDevice.h"
#include "sycamore/utils/Listener.h"

namespace sycamore
{

class Pointer final : public InputDevice<Pointer>
{
public:
    /**
     * @brief Constructor
     */
    explicit Pointer(wlr_input_device* baseHandle);

    /**
     * @brief Destructor
     */
    ~Pointer();

    /**
     * @brief Return ture if this is a touchpad
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

    Listener     m_motion;
    Listener     m_motionAbsolute;
    Listener     m_button;
    Listener     m_axis;
    Listener     m_frame;

    Listener     m_swipeBegin;
    Listener     m_swipeUpdate;
    Listener     m_swipeEnd;

    Listener     m_pinchBegin;
    Listener     m_pinchUpdate;
    Listener     m_pinchEnd;

    Listener     m_holdBegin;
    Listener     m_holdEnd;

    Listener     m_destroy;
};

}

#endif //SYCAMORE_POINTER_H