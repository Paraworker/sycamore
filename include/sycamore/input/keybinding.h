#ifndef SYCAMORE_KEYBINDING_H
#define SYCAMORE_KEYBINDING_H

#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

typedef struct Keybinding              Keybinding;
typedef struct KeybindingManager       KeybindingManager;
typedef struct KeybindingModifiersNode KeybindingModifiersNode;

typedef void (*KeybindingAction)(Keybinding *keybinding);

struct KeybindingManager {
    struct wl_list modifiersNodes;
};

struct KeybindingModifiersNode {
    struct wl_list link;
    struct wl_list keybindings;
    uint32_t       modifiers;

    KeybindingManager *manager;
};

struct Keybinding {
    struct wl_list link;
    uint32_t       modifiers;
    xkb_keysym_t   sym;

    KeybindingAction action;

    KeybindingModifiersNode *modifiersNode;
};

KeybindingManager *keybindingManagerCreate();

void keybindingManagerDestroy(KeybindingManager *manager);

bool handleKeybinding(KeybindingManager *manager, uint32_t modifiers, xkb_keysym_t sym);

#endif //SYCAMORE_KEYBINDING_H