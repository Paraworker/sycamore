#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include "sycamore/input/Cursor.h"
#include "sycamore/input/seatInput/SeatInput.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>

namespace sycamore
{

class Toplevel;

class Seat
{
public:
    /**
     * @brief Create Seat
     */
    static Seat* create(wl_display* display, const char* name);

    /**
     * @brief Get wlr_seat
     */
    auto getHandle() noexcept
    {
        return m_handle;
    }

    template<typename T, typename... Args>
    void setInput(Args&&... args)
    {
        input->onDisable();
        input.reset(new T{std::forward<Args>(args)...});
        input->onEnable();
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
    Seat(wl_display* display, const char* name);

    /**
     * @brief Destructor
     */
    ~Seat();

public:
    Cursor                     cursor;
    std::unique_ptr<SeatInput> input;

private:
    wlr_seat* m_handle;

    Listener  m_setCursor;
    Listener  m_setSelection;
    Listener  m_setPrimarySelection;
    Listener  m_requestStartDrag;
    Listener  m_startDrag;
    Listener  m_destroy;
};

}

#endif //SYCAMORE_SEAT_H