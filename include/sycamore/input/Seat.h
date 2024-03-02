#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include "sycamore/input/seatInput/SeatInput.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>

namespace sycamore
{

class DragIcon;
class Toplevel;

class Seat
{
public:
    /**
     * @brief Constructor
     */
    Seat(wl_display* display, const char* name);

    /**
     * @brief Destructor
     */
    ~Seat();

    /**
     * @brief Get wlr_seat
     */
    auto handle()
    {
        return m_handle;
    }

    void setCapabilities(uint32_t caps);

    void enablePointer();

    void disablePointer();

    bool isPointerEnabled() const
    {
        return m_pointerEnabled;
    }

    size_t pointerButtonCount() const
    {
        return m_pointerButtonCount;
    }

    void updatePointerButtonCount(wl_pointer_button_state state);

    void updatePointerFocus(uint32_t timeMsec);

    void setKeyboardFocus(wlr_surface* surface) const;

    void updateDragIcons() const;

    bool bindingEnterCheck(const Toplevel& toplevel) const;

    template<typename T, typename... Args>
    void setInput(Args&&... args)
    {
        input->onDisable();
        input.reset(new T{std::forward<Args>(args)...});
        input->onEnable();
    }

    Seat(const Seat&) = delete;
    Seat(Seat&&) = delete;
    Seat& operator=(const Seat&) = delete;
    Seat& operator=(Seat&&) = delete;

private:
    void dragIconUpdatePosition(const DragIcon& icon) const;

public:
    std::unique_ptr<SeatInput> input;

private:
    wlr_seat* m_handle;
    bool      m_pointerEnabled;
    size_t    m_pointerButtonCount;

    Listener  m_setCursor;
    Listener  m_setSelection;
    Listener  m_setPrimarySelection;
    Listener  m_requestStartDrag;
    Listener  m_startDrag;
    Listener  m_destroy;
};

}

#endif //SYCAMORE_SEAT_H