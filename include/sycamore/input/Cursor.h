#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Output;

class Cursor
{
public:
    /**
     * @brief Constructor
     */
    Cursor();

    /**
     * @brief Destructor
     */
    ~Cursor();

    /**
     * @brief Attach output layout
     */
    void init(wlr_output_layout* layout);

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
     * @brief Get cursor position in layout
     */
    Point<double> position() const
    {
        return {m_handle->x, m_handle->y};
    }

    /**
     * @brief Move cursor by a vector in layout
     */
    void move(const Point<double>& delta, wlr_input_device* dev)
    {
        wlr_cursor_move(m_handle, dev, delta.x, delta.y);
    }

    /**
     * @brief Warp cursor to the given position in layout
     */
    bool warp(const Point<double>& pos, wlr_input_device* dev = nullptr) const
    {
        return wlr_cursor_warp(m_handle, dev, pos.x, pos.y);
    }

    /**
     * @brief Warp cursor to the given position in absolute[0, 1] coordinates
     */
    void warpAbsolute(const Point<double>& pos, wlr_input_device* dev) const
    {
        wlr_cursor_warp_absolute(m_handle, dev, pos.x, pos.y);
    }

    /**
     * @brief Warp and reset xcursor image again
     */
    void refreshXcursor();

    /**
     * @brief Update xcursor theme
     */
    void updateXcursorTheme(const char* theme, uint32_t size);

    Output* atOutput() const;

    Cursor(const Cursor&) = delete;
    Cursor(Cursor&&) = delete;
    Cursor& operator=(const Cursor&) = delete;
    Cursor& operator=(Cursor&&) = delete;

private:
    wlr_cursor*          m_handle;
    wlr_xcursor_manager* m_xcursorManager;
    const char*          m_xcursor;
};

}

#endif //SYCAMORE_CURSOR_H