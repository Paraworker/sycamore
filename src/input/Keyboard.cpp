#include "sycamore/input/Keyboard.h"

#include "sycamore/input/InputManager.h"
#include "sycamore/input/KeybindingManager.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Keyboard::Keyboard(wlr_input_device* deviceHandle)
    : InputDevice{deviceHandle}
    , m_keyboardHandle{wlr_keyboard_from_input_device(deviceHandle)}
{
    spdlog::info("New Keyboard: {}", deviceHandle->name);

    m_modifiers.notify([this](auto)
    {
        auto seatHandle = core.seat->getHandle();
        wlr_seat_set_keyboard(seatHandle, m_keyboardHandle);
        wlr_seat_keyboard_notify_modifiers(seatHandle, &m_keyboardHandle->modifiers);

        inputManager.syncKeyboardLeds(*this);
    });
    m_modifiers.connect(m_keyboardHandle->events.modifiers);

    m_key.notify([this](void* data)
    {
        auto event = static_cast<wlr_keyboard_key_event*>(data);

        // Translate libinput keycode -> xkbcommon
        uint32_t keycode = event->keycode + 8;
        // Get a list of keysyms based on the keymap for this keyboard
        const xkb_keysym_t* syms;
        int nsyms = xkb_state_key_get_syms(m_keyboardHandle->xkb_state, keycode, &syms);

        // Handle keybinding
        bool handled{false};
        if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            KeyInfo info{getModifiers(), {}};

            for (int i = 0; i < nsyms; ++i)
            {
                info.sym = syms[i];
                handled = keybindingManager.dispatch(info);
            }
        }

        if (!handled)
        {
            auto seatHandle = core.seat->getHandle();
            wlr_seat_set_keyboard(seatHandle, m_keyboardHandle);
            wlr_seat_keyboard_notify_key(seatHandle, event->time_msec, event->keycode, event->state);

            inputManager.syncKeyboardLeds(*this);
        }
    });
    m_key.connect(m_keyboardHandle->events.key);

    m_destroy.notify([this](auto)
    {
        inputManager.removeDevice(this);
    });
    m_destroy.connect(deviceHandle->events.destroy);

    apply();
}

Keyboard::~Keyboard() = default;

void Keyboard::apply()
{
    /* Compile an XKB keymap
    * We assume the defaults right now (e.g. layout = "us"). */
    auto ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx)
    {
        spdlog::error("Create xkb_context failed");
        return;
    }

    auto keymap = xkb_keymap_new_from_names(ctx, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap)
    {
        spdlog::error("Create keymap failed");
        xkb_context_unref(ctx);
        return;
    }

    wlr_keyboard_set_keymap(m_keyboardHandle, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
}

uint32_t Keyboard::ledsState() const
{
    if (!m_keyboardHandle->xkb_state)
    {
        throw std::runtime_error{"Keyboard without xkb_state!"};
    }

    uint32_t leds{0};

    for (uint32_t i = 0; i < WLR_LED_COUNT; ++i)
    {
        if (xkb_state_led_index_is_active(m_keyboardHandle->xkb_state, m_keyboardHandle->led_indexes[i]))
        {
            leds |= (1 << i);
        }
    }

    return leds;
}

}