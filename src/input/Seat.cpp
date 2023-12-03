#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/InputDevice.h"
#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Seat* Seat::create(wl_display* display, wlr_output_layout* layout, const char* name)
{
    auto handle = wlr_seat_create(display, name);
    if (!handle)
    {
        spdlog::error("Create wlr_seat failed");
        return {};
    }

    auto cursor = Cursor::create(layout);
    if (!cursor)
    {
        spdlog::error("Create Cursor failed");
        wlr_seat_destroy(handle);
        return {};
    }

    return new Seat{handle, cursor};
}

Seat::Seat(wlr_seat* handle, Cursor* cursor)
    : m_handle{handle}
    , m_cursor{cursor}
    , m_input{new DefaultInput{*this}}
{
    // Attach cursor
    m_cursor->attachSeat(this);

    // Enable DefaultInput
    m_input->onEnable();

    m_setCursor
    .connect(handle->events.request_set_cursor)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);

        auto* focusedClient = m_handle->pointer_state.focused_client;
        if (focusedClient != event->seat_client)
        {
            return;
        }

        m_cursor->setSurface(event->surface, {event->hotspot_x, event->hotspot_y});
    });

    m_setSelection
    .connect(handle->events.request_set_selection)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_selection_event*>(data);
        wlr_seat_set_selection(m_handle, event->source, event->serial);
    });

    m_setPrimarySelection
    .connect(handle->events.request_set_primary_selection)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_primary_selection_event*>(data);
        wlr_seat_set_primary_selection(m_handle, event->source, event->serial);
    });

    m_requestStartDrag
    .connect(handle->events.request_start_drag)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_request_start_drag_event*>(data);

        if (wlr_seat_validate_pointer_grab_serial(m_handle, event->origin, event->serial))
        {
            wlr_seat_start_pointer_drag(m_handle, event->drag, event->serial);
            return;
        }

        wlr_touch_point* point;
        if (wlr_seat_validate_touch_grab_serial(m_handle, event->origin, event->serial, &point))
        {
            wlr_seat_start_touch_drag(m_handle, event->drag, event->serial, point);
            return;
        }

        // TODO: tablet grabs

        wlr_data_source_destroy(event->drag->source);
    });

    m_startDrag
    .connect(handle->events.start_drag)
    .set([this](void* data)
    {
        auto drag = static_cast<wlr_drag*>(data);

        // Setup icon if provided
        if (drag->icon)
        {
            DragIcon::create(drag->icon, *this);
        }

        setInput(new DefaultInput{*this});
    });

    m_destroy
    .connect(handle->events.destroy)
    .set([this](void*)
    {
        delete this;
    });
}

Seat::~Seat()
{
    m_input->onDisable();
    delete m_input;
    delete m_cursor;
}

void Seat::setCapabilities(uint32_t caps)
{
    wlr_seat_set_capabilities(m_handle, caps);

    // Disable cursor if seat doesn't have pointer capability.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0)
    {
        m_cursor->disable();
    }
}

void Seat::updatePointerFocus(uint32_t timeMsec)
{
    Point<double> sCoords{};
    auto surface = Scene::surfaceFromNode(Core::get().scene->nodeAt(m_cursor->getPosition(), sCoords));

    if (!surface)
    {
        wlr_seat_pointer_clear_focus(m_handle);
        m_cursor->setXcursor("left_ptr");
        return;
    }

    wlr_seat_pointer_notify_enter(m_handle, surface, sCoords.x, sCoords.y);
    wlr_seat_pointer_notify_motion(m_handle, timeMsec, sCoords.x, sCoords.y);
}

void Seat::setKeyboardFocus(wlr_surface* surface) const
{
    auto keyboard = wlr_seat_get_keyboard(m_handle);
    if (!keyboard)
    {
        wlr_seat_keyboard_notify_enter(m_handle, surface, nullptr, 0, nullptr);
        return;
    }

    wlr_seat_keyboard_notify_enter(m_handle, surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

bool Seat::bindingEnterCheck(Toplevel* toplevel) const
{
    /* This function is used for checking whether an
    * 'pointer interactive' input mode should begin. including:
    *
    * PointerMove
    * PointerResize
    */

    if (m_input->type() == SeatInput::BINDING)
    {
        return false;
    }

    if (toplevel->isPinned())
    {
        return false;
    }

    // Deny pointerMove/pointerResize from unfocused toplevel or there is no focused toplevel.
    if (toplevel != Core::get().shell->getFocusState().toplevel)
    {
        return false;
    }

    return true;
}

NAMESPACE_SYCAMORE_END