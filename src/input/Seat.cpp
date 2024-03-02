#include "sycamore/input/Seat.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/input/DragIcon.h"
#include "sycamore/input/seatInput/DefaultInput.h"
#include "sycamore/Core.h"

namespace sycamore
{

Seat::Seat(wl_display* display, const char* name)
    : input{std::make_unique<DefaultInput>(*this)}
    , m_handle{wlr_seat_create(display, name)}
    , m_pointerEnabled{false}
    , m_pointerButtonCount{0}
{
    m_setCursor = [this](void* data)
    {
        auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);

        auto* focusedClient = m_handle->pointer_state.focused_client;
        if (focusedClient == event->seat_client)
        {
            core.cursor.setSurface(event->surface, {event->hotspot_x, event->hotspot_y});
        }
    };
    m_setCursor.connect(m_handle->events.request_set_cursor);

    m_setSelection = [this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_selection_event*>(data);
        wlr_seat_set_selection(m_handle, event->source, event->serial);
    };
    m_setSelection.connect(m_handle->events.request_set_selection);

    m_setPrimarySelection = [this](void* data)
    {
        auto event = static_cast<wlr_seat_request_set_primary_selection_event*>(data);
        wlr_seat_set_primary_selection(m_handle, event->source, event->serial);
    };
    m_setPrimarySelection.connect(m_handle->events.request_set_primary_selection);

    m_requestStartDrag = [this](void* data)
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
    };
    m_requestStartDrag.connect(m_handle->events.request_start_drag);

    m_startDrag = [this](void* data)
    {
        auto drag = static_cast<wlr_drag*>(data);

        // Setup icon if provided
        if (drag->icon)
        {
            auto icon = new DragIcon{drag->icon};
            dragIconUpdatePosition(*icon);
        }

        setInput<DefaultInput>(*this);
    };
    m_startDrag.connect(m_handle->events.start_drag);

    m_destroy = [](auto)
    {
        core.seat.reset();
    };
    m_destroy.connect(m_handle->events.destroy);
}

Seat::~Seat() = default;

void Seat::setCapabilities(uint32_t caps)
{
    wlr_seat_set_capabilities(m_handle, caps);

    // Disable pointer if seat doesn't have pointer capability.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0)
    {
        disablePointer();
    }
}

void Seat::enablePointer()
{
    if (m_pointerEnabled)
    {
        return;
    }

    m_pointerEnabled = true;

    input->rebasePointer();
}

void Seat::disablePointer()
{
    if (!m_pointerEnabled)
    {
        return;
    }

    // Hide cursor
    core.cursor.hide();

    // Clear pointer focus
    wlr_seat_pointer_notify_clear_focus(m_handle);

    m_pointerEnabled = false;
}

void Seat::updatePointerButtonCount(wl_pointer_button_state state)
{
    if (state == WL_POINTER_BUTTON_STATE_PRESSED)
    {
        ++m_pointerButtonCount;
    }
    else if (m_pointerButtonCount > 0)
    {
        --m_pointerButtonCount;
    }
}

void Seat::updatePointerFocus(uint32_t timeMsec)
{
    if (!m_pointerEnabled)
    {
        return;
    }

    Point<double> sCoords{};
    auto surface = scene::surfaceFromNode(core.scene.shellAt(core.cursor.position(), sCoords));

    if (!surface)
    {
        wlr_seat_pointer_notify_clear_focus(m_handle);
        core.cursor.setXcursor("left_ptr");
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

void Seat::updateDragIcons() const
{
    wlr_scene_node* node;
    wl_list_for_each(node, &core.scene.dragIcons->children, link)
    {
        dragIconUpdatePosition(static_cast<DragIconElement*>(node->data)->icon);
    }
}

bool Seat::bindingEnterCheck(const Toplevel& toplevel) const
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

    if (toplevel.isPinned())
    {
        return false;
    }

    // Deny pointerMove/pointerResize for unfocused toplevel
    // or there is no focused toplevel.
    if (windowManager.focusState().toplevel != &toplevel)
    {
        return false;
    }

    return true;
}

void Seat::dragIconUpdatePosition(const DragIcon& icon) const
{
    switch (icon.grabType())
    {
        case WLR_DRAG_GRAB_KEYBOARD:
            return;
        case WLR_DRAG_GRAB_KEYBOARD_POINTER:
            icon.setPosition(static_cast<Point<int32_t>>(core.cursor.position()));
            break;
        case WLR_DRAG_GRAB_KEYBOARD_TOUCH:
            // TODO
            break;
    }
}

}