#include "sycamore/input/Cursor.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <string>

NAMESPACE_SYCAMORE_BEGIN

static wlr_xcursor_manager* xcursorManagerCreate(const char* theme, uint32_t size)
{
    if (theme)
    {
        setenv("XCURSOR_THEME", theme, 1);
    }

    setenv("XCURSOR_SIZE", std::to_string(size).c_str(), 1);

    return wlr_xcursor_manager_create(theme, size);
}

Cursor* Cursor::create(wlr_output_layout* layout)
{
    auto handle = wlr_cursor_create();
    if (!handle)
    {
        return {};
    }

    auto manager = xcursorManagerCreate(nullptr, 24);
    if (!manager)
    {
        wlr_cursor_destroy(handle);
        return {};
    }

    wlr_cursor_attach_output_layout(handle, layout);

    return new Cursor{handle, manager};
}

Cursor::Cursor(wlr_cursor* handle, wlr_xcursor_manager* manager)
    : m_handle{handle}
    , m_xcursorManager{manager}
    , m_enabled{false}
    , m_xcursor{nullptr}
    , m_pointerButtonCount{0}
    , m_seat{nullptr}
{
    m_motion
    .connect(handle->events.motion)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_event*>(data);

        enable();

        wlr_cursor_move(m_handle, &event->pointer->base, event->delta_x, event->delta_y);
        m_seat->getInput().onPointerMotion(event->time_msec);
    });

    m_motionAbsolute
    .connect(handle->events.motion_absolute)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);

        enable();

        wlr_cursor_warp_absolute(m_handle, &event->pointer->base, event->x, event->y);
        m_seat->getInput().onPointerMotion(event->time_msec);
    });

    m_button
    .connect(handle->events.button)
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

        m_seat->getInput().onPointerButton(event);
    });

    m_axis
    .connect(handle->events.axis)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerAxis(static_cast<wlr_pointer_axis_event*>(data));
    });

    m_frame
    .connect(handle->events.frame)
    .set([this](void* data)
    {
        enable();
        wlr_seat_pointer_notify_frame(m_seat->getHandle());
    });

    m_swipeBegin
    .connect(handle->events.swipe_begin)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerSwipeBegin(static_cast<wlr_pointer_swipe_begin_event*>(data));
    });

    m_swipeUpdate
    .connect(handle->events.swipe_update)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerSwipeUpdate(static_cast<wlr_pointer_swipe_update_event*>(data));
    });

    m_swipeEnd
    .connect(handle->events.swipe_end)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerSwipeEnd(static_cast<wlr_pointer_swipe_end_event*>(data));
    });

    m_pinchBegin
    .connect(handle->events.pinch_begin)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerPinchBegin(static_cast<wlr_pointer_pinch_begin_event*>(data));
    });

    m_pinchUpdate
    .connect(handle->events.pinch_update)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerPinchUpdate(static_cast<wlr_pointer_pinch_update_event*>(data));
    });

    m_pinchEnd
    .connect(handle->events.pinch_end)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerPinchEnd(static_cast<wlr_pointer_pinch_end_event*>(data));
    });

    m_holdBegin
    .connect(handle->events.hold_begin)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerHoldBegin(static_cast<wlr_pointer_hold_begin_event*>(data));
    });

    m_holdEnd
    .connect(handle->events.hold_end)
    .set([this](void* data)
    {
        enable();
        m_seat->getInput().onPointerHoldEnd(static_cast<wlr_pointer_hold_end_event*>(data));
    });
}

Cursor::~Cursor()
{
    // Disconnect signals before m_handle destroyed
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

    // Maybe nullptr
    if (m_xcursorManager)
    {
        wlr_xcursor_manager_destroy(m_xcursorManager);
    }

    wlr_cursor_destroy(m_handle);
}

void Cursor::enable()
{
    if (m_enabled)
    {
        return;
    }

    m_enabled = true;

    m_seat->getInput().rebasePointer();
}

void Cursor::disable()
{
    if (!m_enabled)
    {
        return;
    }

    m_enabled = false;

    hide();
    wlr_seat_pointer_notify_clear_focus(m_seat->getHandle());
}

void Cursor::setXcursor(const char* name)
{
    if (!m_enabled)
    {
        return;
    }

    if (!name)
    {
        hide();
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

    wlr_cursor_set_xcursor(m_handle, m_xcursorManager, m_xcursor);
}

bool Cursor::updateXcursorTheme(const char* theme, uint32_t size)
{
    if (m_xcursorManager)
    {
        wlr_xcursor_manager_destroy(m_xcursorManager);
    }

    if (m_xcursorManager = xcursorManagerCreate(theme, size); !m_xcursorManager)
    {
        return false;
    }

    refreshXcursor();

    return true;
}

Output* Cursor::atOutput() const
{
    return Core::get().outputLayout->findOutputAt(getPosition());
}

NAMESPACE_SYCAMORE_END