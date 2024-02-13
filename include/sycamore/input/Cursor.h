#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Output;
class Seat;

class Cursor
{
public:
    /**
     * @brief Constructor
     */
    explicit Cursor(Seat& seat);

    /**
     * @brief Destructor
     */
    ~Cursor();

    void init(wlr_output_layout* layout);

    void enable();

    void disable();

    bool isEnabled() const
    {
        return m_enabled;
    }

    /**
     * @brief Set cursor image by xcursor name
     */
    void setXcursor(const char* name);

    /**
     * @brief Set cursor image by surface
     */
    void setSurface(wlr_surface* surface, const Point<int32_t>& hotspot);

    /**
     * @brief Hide cursor
     */
    void hide();

    /**
     * @brief Warp and reset xcursor image again
     */
    void refreshXcursor();

    /**
     * @brief Update xcursor theme
     */
    void updateXcursorTheme(const char* theme, uint32_t size);

    Output* atOutput() const;

    void warp(const Point<double>& coords) const
    {
        wlr_cursor_warp(m_handle, nullptr, coords.x, coords.y);
    }

    Point<double> position() const
    {
        return {m_handle->x, m_handle->y};
    }

    size_t pointerButtonCount() const
    {
        return m_pointerButtonCount;
    }

    void attachDevice(wlr_input_device* device) const
    {
        wlr_cursor_attach_input_device(m_handle, device);
    }

    void detachDevice(wlr_input_device* device) const
    {
        wlr_cursor_detach_input_device(m_handle, device);
    }

    Cursor(const Cursor&) = delete;
    Cursor(Cursor&&) = delete;
    Cursor& operator=(const Cursor&) = delete;
    Cursor& operator=(Cursor&&) = delete;

private:
    wlr_cursor*          m_handle;
    wlr_xcursor_manager* m_xcursorManager;
    bool                 m_enabled;
    const char*          m_xcursor;
    size_t               m_pointerButtonCount;
    Seat&                m_seat;

    Listener             m_motion;
    Listener             m_motionAbsolute;
    Listener             m_button;
    Listener             m_axis;
    Listener             m_frame;
    Listener             m_swipeBegin;
    Listener             m_swipeUpdate;
    Listener             m_swipeEnd;
    Listener             m_pinchBegin;
    Listener             m_pinchUpdate;
    Listener             m_pinchEnd;
    Listener             m_holdBegin;
    Listener             m_holdEnd;
};

}

#endif //SYCAMORE_CURSOR_H