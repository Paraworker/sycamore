#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include "sycamore/utils/Listener.h"
#include "sycamore/utils/CHandle.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <memory>

namespace sycamore
{

class Output;
class Seat;

class Cursor
{
public:
    /**
     * @brief Constructor
     * @throw std::runtime_error on failure
     */
    Cursor(wlr_output_layout* layout, Seat& seat);

    /**
     * @brief Destructor
     */
    ~Cursor();

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
    bool updateXcursorTheme(const char* theme, uint32_t size);

    Output* atOutput() const;

    void warp(const Point<double>& coords) const
    {
        wlr_cursor_warp(m_handle.get(), nullptr, coords.x, coords.y);
    }

    Point<double> getPosition() const
    {
        return {m_handle->x, m_handle->y};
    }

    size_t getPointerButtonCount() const
    {
        return m_pointerButtonCount;
    }

    void attachDevice(wlr_input_device* device) const
    {
        wlr_cursor_attach_input_device(m_handle.get(), device);
    }

    void detachDevice(wlr_input_device* device) const
    {
        wlr_cursor_detach_input_device(m_handle.get(), device);
    }

    Cursor(const Cursor&) = delete;
    Cursor(Cursor&&) = delete;
    Cursor& operator=(const Cursor&) = delete;
    Cursor& operator=(Cursor&&) = delete;

private:
    using Handle               = CHandle<wlr_cursor, wlr_cursor_destroy>;
    using XcursorManagerHandle = CHandle<wlr_xcursor_manager, wlr_xcursor_manager_destroy>;

private:
    Handle               m_handle;
    XcursorManagerHandle m_xcursorManager;
    bool                 m_enabled;
    const char*          m_xcursor;
    size_t               m_pointerButtonCount;
    Seat&                m_seat;

private: // Declare listeners at last, so they can destruct first
    Listener m_motion;
    Listener m_motionAbsolute;
    Listener m_button;
    Listener m_axis;
    Listener m_frame;
    Listener m_swipeBegin;
    Listener m_swipeUpdate;
    Listener m_swipeEnd;
    Listener m_pinchBegin;
    Listener m_pinchUpdate;
    Listener m_pinchEnd;
    Listener m_holdBegin;
    Listener m_holdEnd;
};

}

#endif //SYCAMORE_CURSOR_H