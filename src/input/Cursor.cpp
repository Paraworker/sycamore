#include "sycamore/input/Cursor.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <stdexcept>
#include <string>

namespace sycamore
{

static wlr_xcursor_manager* createXcursorManager(const char* theme, uint32_t size)
{
    if (theme)
    {
        setenv("XCURSOR_THEME", theme, true);
    }

    setenv("XCURSOR_SIZE", std::to_string(size).c_str(), true);

    return wlr_xcursor_manager_create(theme, size);
}

Cursor::Cursor(wlr_output_layout* layout, Seat& seat)
    : m_handle{wlr_cursor_create()}
    , m_xcursorManager{createXcursorManager(nullptr, 24)}
    , m_enabled{false}
    , m_xcursor{}
    , m_pointerButtonCount{0}
    , m_seat{seat}
{
    if (!m_handle)
    {
        throw std::runtime_error("Create wlr_cursor failed!");
    }

    if (!m_xcursorManager)
    {
        throw std::runtime_error("Create wlr_xcursor_manager failed!");
    }

    wlr_cursor_attach_output_layout(m_handle.get(), layout);

    m_motion
    .connect(m_handle->events.motion)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_event*>(data);

        enable();

        wlr_cursor_move(m_handle.get(), &event->pointer->base, event->delta_x, event->delta_y);
        m_seat.getInput().onPointerMotion(event->time_msec);
    });

    m_motionAbsolute
    .connect(m_handle->events.motion_absolute)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);

        enable();

        wlr_cursor_warp_absolute(m_handle.get(), &event->pointer->base, event->x, event->y);
        m_seat.getInput().onPointerMotion(event->time_msec);
    });

    m_button
    .connect(m_handle->events.button)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_pointer_button_event*>(data);

        enable();

        if (event->state == WLR_BUTTON_PRESSED)
        {
            ++m_pointerButtonCount;
        }
        else if (m_pointerButtonCount > 0)
        {
            --m_pointerButtonCount;
        }

        m_seat.getInput().onPointerButton(event);
    });

    m_axis
    .connect(m_handle->events.axis)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerAxis(static_cast<wlr_pointer_axis_event*>(data));
    });

    m_frame
    .connect(m_handle->events.frame)
    .set([this](void* data)
    {
        enable();
        wlr_seat_pointer_notify_frame(m_seat.getHandle());
    });

    m_swipeBegin
    .connect(m_handle->events.swipe_begin)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerSwipeBegin(static_cast<wlr_pointer_swipe_begin_event*>(data));
    });

    m_swipeUpdate
    .connect(m_handle->events.swipe_update)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerSwipeUpdate(static_cast<wlr_pointer_swipe_update_event*>(data));
    });

    m_swipeEnd
    .connect(m_handle->events.swipe_end)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerSwipeEnd(static_cast<wlr_pointer_swipe_end_event*>(data));
    });

    m_pinchBegin
    .connect(m_handle->events.pinch_begin)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerPinchBegin(static_cast<wlr_pointer_pinch_begin_event*>(data));
    });

    m_pinchUpdate
    .connect(m_handle->events.pinch_update)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerPinchUpdate(static_cast<wlr_pointer_pinch_update_event*>(data));
    });

    m_pinchEnd
    .connect(m_handle->events.pinch_end)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerPinchEnd(static_cast<wlr_pointer_pinch_end_event*>(data));
    });

    m_holdBegin
    .connect(m_handle->events.hold_begin)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerHoldBegin(static_cast<wlr_pointer_hold_begin_event*>(data));
    });

    m_holdEnd
    .connect(m_handle->events.hold_end)
    .set([this](void* data)
    {
        enable();
        m_seat.getInput().onPointerHoldEnd(static_cast<wlr_pointer_hold_end_event*>(data));
    });
}

Cursor::~Cursor() = default;

void Cursor::enable()
{
    if (m_enabled)
    {
        return;
    }

    m_enabled = true;

    m_seat.getInput().rebasePointer();
}

void Cursor::disable()
{
    if (!m_enabled)
    {
        return;
    }

    // Clear image
    m_xcursor = nullptr;
    wlr_cursor_unset_image(m_handle.get());

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
    wlr_cursor_set_xcursor(m_handle.get(), m_xcursorManager.get(), name);
}

void Cursor::setSurface(wlr_surface* surface, const Point<int32_t>& hotspot)
{
    if (!m_enabled)
    {
        return;
    }

    m_xcursor = nullptr;
    wlr_cursor_set_surface(m_handle.get(), surface, hotspot.x, hotspot.y);
}

void Cursor::hide()
{
    if (!m_enabled)
    {
        return;
    }

    m_xcursor = nullptr;
    wlr_cursor_unset_image(m_handle.get());
}

void Cursor::refreshXcursor()
{
    if (!m_enabled)
    {
        return;
    }

    warp(getPosition());

    if (!m_xcursor)
    {
        return;
    }

    wlr_cursor_set_xcursor(m_handle.get(), m_xcursorManager.get(), m_xcursor);
}

bool Cursor::updateXcursorTheme(const char* theme, uint32_t size)
{
    if (m_xcursorManager.reset(createXcursorManager(theme, size)); !m_xcursorManager)
    {
        return false;
    }

    refreshXcursor();

    return true;
}

Output* Cursor::atOutput() const
{
    return Core::instance.outputLayout->findOutputAt(getPosition());
}

}