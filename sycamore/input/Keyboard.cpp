#include "sycamore/input/InputManager.h"
#include "sycamore/input/Keyboard.h"
#include "sycamore/input/KeybindingManager.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

static xkb_keymap* compileKeymap() {
    /* Compile an XKB keymap
    * We assume the defaults right now (e.g. layout = "us"). */
    auto context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!context) {
        spdlog::error("Create xkb_context failed");
        return nullptr;
    }

    auto keymap = xkb_keymap_new_from_names(context, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkb_context_unref(context);
    return keymap;
}

Keyboard* Keyboard::create(wlr_input_device* deviceHandle) {
    spdlog::info("New Keyboard: {}", deviceHandle->name);

    auto keyboardHandle = wlr_keyboard_from_input_device(deviceHandle);

    auto keymap = compileKeymap();
    if (!keymap) {
        spdlog::error("Keyboard: {} compile xkb_keymap failed", deviceHandle->name);
        return nullptr;
    }

    if (!wlr_keyboard_set_keymap(keyboardHandle, keymap)) {
        spdlog::error("Keyboard: {} set keymap failed", deviceHandle->name);
        xkb_keymap_unref(keymap);
        return nullptr;
    }

    xkb_keymap_unref(keymap);

    auto keyboard = new Keyboard{deviceHandle, keyboardHandle};

    InputManager::instance.add(keyboard);

    return keyboard;
}

Keyboard::Keyboard(wlr_input_device* deviceHandle, wlr_keyboard* keyboardHandle)
    : InputDevice(deviceHandle), m_keyboardHandle(keyboardHandle) {
    m_modifiers.set(&keyboardHandle->events.modifiers, [this](void*) {
        auto seatHandle = Core::instance.seat->getHandle();
        wlr_seat_set_keyboard(seatHandle, m_keyboardHandle);
        wlr_seat_keyboard_notify_modifiers(seatHandle, &m_keyboardHandle->modifiers);

        syncLeds();
    });

    m_key.set(&keyboardHandle->events.key, [this](void* data) {
        auto event = static_cast<wlr_keyboard_key_event*>(data);

        // Translate libinput keycode -> xkbcommon
        uint32_t keycode = event->keycode + 8;
        // Get a list of keysyms based on the keymap for this keyboard
        const xkb_keysym_t* syms;
        int nsyms = xkb_state_key_get_syms(m_keyboardHandle->xkb_state, keycode, &syms);

        // Handle keybinding
        bool handled{false};
        if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            KeyInfo info{getModifiers(), {}};

            for (int i = 0; i < nsyms; ++i) {
                info.sym = syms[i];
                handled = KeybindingManager::instance.dispatch(info);
            }
        }

        if (!handled) {
            auto seatHandle = Core::instance.seat->getHandle();
            wlr_seat_set_keyboard(seatHandle, m_keyboardHandle);
            wlr_seat_keyboard_notify_key(seatHandle, event->time_msec, event->keycode, event->state);

            syncLeds();
        }
    });

    m_destroy.set(&deviceHandle->events.destroy, [this](void*) {
        InputManager::instance.remove(this);
        delete this;
    });
}

Keyboard::~Keyboard() = default;

void Keyboard::syncLeds() {
    if (!m_keyboardHandle->xkb_state) {
        return;
    }

    uint32_t leds = 0;
    for (uint32_t i = 0; i < WLR_LED_COUNT; ++i) {
        if (xkb_state_led_index_is_active(m_keyboardHandle->xkb_state, m_keyboardHandle->led_indexes[i])) {
            leds |= (1 << i);
        }
    }

    Keyboard* keyboard;
    wl_list_for_each(keyboard, &InputManager::instance.getDeviceList(WLR_INPUT_DEVICE_KEYBOARD).getHandle(), link) {
        if (keyboard->m_keyboardHandle != m_keyboardHandle) {
            wlr_keyboard_led_update(keyboard->m_keyboardHandle, leds);
        }
    }
}

NAMESPACE_SYCAMORE_END