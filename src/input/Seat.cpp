#include "sycamore/input/Seat.h"

#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/Core.h"

#include <stdexcept>

namespace sycamore
{

Seat* Seat::create(wl_display* display, const char* name, wlr_output_layout* layout)
{
    // Will be destroyed by listener
    return new Seat{display, name, layout};
}

Seat::Seat(wl_display* display, const char* name, wlr_output_layout* layout)
    : cursor{layout, *this}
    , input{std::make_unique<DefaultInput>(*this)}
    , m_handle{nullptr}
{
    if (m_handle = wlr_seat_create(display, name); !m_handle)
    {
        throw std::runtime_error("Create wlr_seat failed!");
    }

    m_setCursor
    .connect(m_handle->events.request_set_cursor)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);

        auto* focusedClient = m_handle->pointer_state.focused_client;
        if (focusedClient != event->seat_client)
        {
            return;
        }

        cursor.setSurface(event->surface, {event->hotspot_x, event->hotspot_y});
    });

    m_setSelection
    .connect(m_handle->events.request_set_selection)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_selection_event*>(data);
        wlr_seat_set_selection(m_handle, event->source, event->serial);
    });

    m_setPrimarySelection
    .connect(m_handle->events.request_set_primary_selection)
    .set([this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_primary_selection_event*>(data);
        wlr_seat_set_primary_selection(m_handle, event->source, event->serial);
    });

    m_requestStartDrag
    .connect(m_handle->events.request_start_drag)
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
    .connect(m_handle->events.start_drag)
    .set([this](void* data)
    {
        auto drag = static_cast<wlr_drag*>(data);

        // Setup icon if provided
        if (drag->icon)
        {
            DragIcon::create(drag->icon, *this);
        }

        setInput<DefaultInput>(*this);
    });

    m_destroy
    .connect(m_handle->events.destroy)
    .set([this](auto)
    {
        delete this;
    });
}

Seat::~Seat() = default;

void Seat::setCapabilities(uint32_t caps)
{
    wlr_seat_set_capabilities(m_handle, caps);

    // Disable cursor if seat doesn't have pointer capability.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0)
    {
        cursor.disable();
    }
}

void Seat::updatePointerFocus(uint32_t timeMsec)
{
    Point<double> sCoords{};
    auto surface = Scene::surfaceFromNode(Core::instance.scene->nodeAt(cursor.getPosition(), sCoords));

    if (!surface)
    {
        wlr_seat_pointer_clear_focus(m_handle);
        cursor.setXcursor("left_ptr");
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

    if (input->type() == SeatInput::BINDING)
    {
        return false;
    }

    if (toplevel->isPinned())
    {
        return false;
    }

    // Deny pointerMove/pointerResize from unfocused toplevel or there is no focused toplevel.
    if (toplevel != ShellManager::instance.getFocusState().toplevel)
    {
        return false;
    }

    return true;
}

}