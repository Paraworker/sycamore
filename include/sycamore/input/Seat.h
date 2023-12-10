#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include "sycamore/input/Cursor.h"
#include "sycamore/input/seatInput/SeatInput.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Toplevel;

class Seat
{
public:
    /**
     * @brief Create default Seat
     * @throw std::runtime_error on failure
     * @lifetime Be destroyed by listener
     */
    static Seat* createDefault(wl_display* display, wlr_output_layout* layout);

    auto getHandle() const
    {
        return m_handle;
    }

    Cursor& getCursor()
    {
        return m_cursor;
    }

    SeatInput& getInput()
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
    /**
     * @brief Constructor
     */
    Seat(wl_display* display, wlr_output_layout* layout, const char* name);

    /**
     * @brief Destructor
     */
    ~Seat();

private:
    wlr_seat*  m_handle;
    SeatInput* m_input;
    Cursor     m_cursor;

private:
    Listener m_setCursor;
    Listener m_setSelection;
    Listener m_setPrimarySelection;
    Listener m_requestStartDrag;
    Listener m_startDrag;
    Listener m_destroy;
};

}

#endif //SYCAMORE_SEAT_H