#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include "sycamore/defines.h"
#include "sycamore/input/Cursor.h"
#include "sycamore/input/seatInput/SeatInput.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>

NAMESPACE_SYCAMORE_BEGIN

class Toplevel;

class Seat
{
public:
    using UPtr = std::unique_ptr<Seat>;
    
public:
    /**
     * @brief Create Seat
     * @return nullptr on failure
     */
    static Seat::UPtr create(wl_display* display, wlr_output_layout* layout, const char* name);

    ~Seat();

    auto getHandle() const
    {
        return m_handle;
    }

    Cursor& getCursor() const
    {
        return *m_cursor;
    }

    SeatInput& getInput() const
    {
        return *m_input;
    }

    void setInput(SeatInput* newInput)
    {
        m_input->onDisable();
        delete m_input;

        m_input = newInput;
        newInput->onEnable();
    }

    void updatePointerFocus(uint32_t timeMsec);

    void setKeyboardFocus(wlr_surface* surface) const;

    void setCapabilities(uint32_t caps);

    bool bindingEnterCheck(Toplevel* toplevel) const;

    Seat(const Seat&) = delete;
    Seat(Seat&&) = delete;
    Seat& operator=(const Seat&) = delete;
    Seat& operator=(Seat&&) = delete;

private:
    Seat(wlr_seat* handle, Cursor* cursor);

private:
    wlr_seat*  m_handle;
    Cursor*    m_cursor;
    SeatInput* m_input;

private:
    Listener m_setCursor;
    Listener m_setSelection;
    Listener m_setPrimarySelection;
    Listener m_requestStartDrag;
    Listener m_startDrag;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SEAT_H