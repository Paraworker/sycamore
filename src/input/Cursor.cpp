#include "sycamore/input/Cursor.h"

#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputManager.h"

#include <string>

namespace sycamore
{

static wlr_xcursor_manager* createXcursorManager(const char* theme = nullptr, uint32_t size = 24)
{
    if (theme)
    {
        setenv("XCURSOR_THEME", theme, true);
    }

    setenv("XCURSOR_SIZE", std::to_string(size).c_str(), true);

    return wlr_xcursor_manager_create(theme, size);
}

Cursor::Cursor(Seat& seat)
    : m_handle{wlr_cursor_create()}
    , m_xcursorManager{createXcursorManager()}
    , m_enabled{false}
    , m_xcursor{}
    , m_pointerButtonCount{0}
    , m_seat{seat}
{
    m_motion = [this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_event*>(data);

        enable();

        wlr_cursor_move(m_handle, &event->pointer->base, event->delta_x, event->delta_y);
        m_seat.input->onPointerMotion(event->time_msec);
    };
    m_motion.connect(m_handle->events.motion);

    m_motionAbsolute = [this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);

        enable();

        wlr_cursor_warp_absolute(m_handle, &event->pointer->base, event->x, event->y);
        m_seat.input->onPointerMotion(event->time_msec);
    };
    m_motionAbsolute.connect(m_handle->events.motion_absolute);

    m_button = [this](void* data)
    {
        auto event = static_cast<wlr_pointer_button_event*>(data);

        enable();

        if (event->state == WL_POINTER_BUTTON_STATE_PRESSED)
        {
            ++m_pointerButtonCount;
        }
        else if (m_pointerButtonCount > 0)
        {
            --m_pointerButtonCount;
        }

        m_seat.input->onPointerButton(event);
    };
    m_button.connect(m_handle->events.button);

    m_axis = [this](void* data)
    {
        enable();
        m_seat.input->onPointerAxis(static_cast<wlr_pointer_axis_event*>(data));
    };
    m_axis.connect(m_handle->events.axis);

    m_frame = [this](auto)
    {
        enable();
        wlr_seat_pointer_notify_frame(m_seat.getHandle());
    };
    m_frame.connect(m_handle->events.frame);

    m_swipeBegin = [this](void* data)
    {
        enable();
        m_seat.input->onPointerSwipeBegin(static_cast<wlr_pointer_swipe_begin_event*>(data));
    };
    m_swipeBegin.connect(m_handle->events.swipe_begin);

    m_swipeUpdate = [this](void* data)
    {
        enable();
        m_seat.input->onPointerSwipeUpdate(static_cast<wlr_pointer_swipe_update_event*>(data));
    };
    m_swipeUpdate.connect(m_handle->events.swipe_update);

    m_swipeEnd = [this](void* data)
    {
        enable();
        m_seat.input->onPointerSwipeEnd(static_cast<wlr_pointer_swipe_end_event*>(data));
    };
    m_swipeEnd.connect(m_handle->events.swipe_end);

    m_pinchBegin = [this](void* data)
    {
        enable();
        m_seat.input->onPointerPinchBegin(static_cast<wlr_pointer_pinch_begin_event*>(data));
    };
    m_pinchBegin.connect(m_handle->events.pinch_begin);

    m_pinchUpdate = [this](void* data)
    {
        enable();
        m_seat.input->onPointerPinchUpdate(static_cast<wlr_pointer_pinch_update_event*>(data));
    };
    m_pinchUpdate.connect(m_handle->events.pinch_update);

    m_pinchEnd = [this](void* data)
    {
        enable();
        m_seat.input->onPointerPinchEnd(static_cast<wlr_pointer_pinch_end_event*>(data));
    };
    m_pinchEnd.connect(m_handle->events.pinch_end);

    m_holdBegin = [this](void* data)
    {
        enable();
        m_seat.input->onPointerHoldBegin(static_cast<wlr_pointer_hold_begin_event*>(data));
    };
    m_holdBegin.connect(m_handle->events.hold_begin);

    m_holdEnd = [this](void* data)
    {
        enable();
        m_seat.input->onPointerHoldEnd(static_cast<wlr_pointer_hold_end_event*>(data));
    };
    m_holdEnd.connect(m_handle->events.hold_end);
}

void Cursor::init(wlr_output_layout* layout)
{
    wlr_cursor_attach_output_layout(m_handle, layout);
}

Cursor::~Cursor()
{
    // Listeners should be disconnected
    // before wlr_cursor is destroyed
    m_motion.disconnect();
    m_motionAbsolute.disconnect();
    m_button.disconnect();
    m_axis.disconnect();
    m_frame.disconnect();
    m_swipeBegin.disconnect();
    m_swipeUpdate.disconnect();
    m_swipeEnd.disconnect();
    m_pinchBegin.disconnect();
    m_pinchUpdate.disconnect();
    m_pinchEnd.disconnect();
    m_holdBegin.disconnect();
    m_holdEnd.disconnect();

    wlr_xcursor_manager_destroy(m_xcursorManager);
    wlr_cursor_destroy(m_handle);
}

void Cursor::enable()
{
    if (m_enabled)
    {
        return;
    }

    m_enabled = true;

    m_seat.input->rebasePointer();
}

void Cursor::disable()
{
    if (!m_enabled)
    {
        return;
    }

    // Clear image
    m_xcursor = nullptr;
    wlr_cursor_unset_image(m_handle);

    // Clear pointer focus
    wlr_seat_pointer_notify_clear_focus(m_seat.getHandle());

    m_enabled = false;
}

void Cursor::setXcursor(const char* name)
{
    if (!m_enabled)
    {
        return;
    }

    m_xcursor = name;
    wlr_cursor_set_xcursor(m_handle, m_xcursorManager, name);
}

void Cursor::setSurface(wlr_surface* surface, const Point<int32_t>& hotspot)
{
    if (!m_enabled)
    {
        return;
    }

    m_xcursor = nullptr;
    wlr_cursor_set_surface(m_handle, surface, hotspot.x, hotspot.y);
}

void Cursor::hide()
{
    if (!m_enabled)
    {
        return;
    }

    m_xcursor = nullptr;
    wlr_cursor_unset_image(m_handle);
}

void Cursor::refreshXcursor()
{
    if (!m_enabled)
    {
        return;
    }

    warp(position());

    if (m_xcursor)
    {
        wlr_cursor_set_xcursor(m_handle, m_xcursorManager, m_xcursor);
    }
}

void Cursor::updateXcursorTheme(const char* theme, uint32_t size)
{
    if (m_xcursorManager)
    {
        wlr_xcursor_manager_destroy(m_xcursorManager);
    }

    m_xcursorManager = createXcursorManager(theme, size);

    refreshXcursor();
}

Output* Cursor::atOutput() const
{
    return OutputManager::findOutputAt(position());
}

}