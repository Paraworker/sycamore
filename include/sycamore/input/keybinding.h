#ifndef SYCAMORE_KEYBINDING_H
#define SYCAMORE_KEYBINDING_H

#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

struct sycamore_keybinding;

typedef void (*keybinding_action)(struct sycamore_keybinding *keybinding);

struct sycamore_keybinding_manager {
    struct wl_list modifiers_nodes;
};

struct keybinding_modifiers_node {
    struct wl_list link;
    struct wl_list keybindings;
    uint32_t modifiers;

    struct sycamore_keybinding_manager *manager;
};

struct sycamore_keybinding {
    struct wl_list link;
    uint32_t modifiers;
    xkb_keysym_t sym;

    keybinding_action action;

    struct keybinding_modifiers_node *modifiers_node;
};

struct sycamore_keybinding_manager *sycamore_keybinding_manager_create();

void sycamore_keybinding_manager_destroy(struct sycamore_keybinding_manager *manager);

bool handle_keybinding(struct sycamore_keybinding_manager *manager,
        uint32_t modifiers, xkb_keysym_t sym);

#endif //SYCAMORE_KEYBINDING_H