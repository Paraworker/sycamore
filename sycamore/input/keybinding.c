#include <stdlib.h>
#include <unistd.h>
#include <wlr/backend/session.h>
#include <wlr/util/log.h>
#include "sycamore/input/keybinding.h"
#include "sycamore/input/seat.h"
#include "sycamore/desktop/view.h"
#include "sycamore/server.h"

bool handleKeybinding(KeybindingManager *manager, uint32_t modifiers, xkb_keysym_t sym) {
    if (modifiers == 0) {
        return false;
    }

    KeybindingModifiersNode *node;
    wl_list_for_each(node, &manager->modifiersNodes, link) {
        if (modifiers == node->modifiers) {
            struct Keybinding *keybinding;
            wl_list_for_each(keybinding, &node->keybindings, link) {
                if (sym == keybinding->sym) {
                    keybinding->action(keybinding);
                    return true;
                }
            } //for each
            return false;
        }
    } //for each
    return false;
}

// action
static void openLauncher(Keybinding *keybinding) {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "fuzzel -i Papirus", (void *)NULL);
    }
}

// action
static void openTerminal(Keybinding *keybinding) {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "gnome-terminal", (void *)NULL);
    }
}

// action
static void closeFocusedView(Keybinding *keybinding) {
    View *view = server.focusedView.view;
    if (view) {
        view->interface->close(view);
    }
}

// action
static void cycleView(Keybinding *keybinding) {
    /* Cycle to the next view */
    if (wl_list_length(&server.mappedViews) < 2) {
        return;
    }

    View *nextView = wl_container_of(server.mappedViews.prev, nextView, link);
    viewSetFocus(nextView);
    seatopPointerRebase(server.seat);
}

// action
static void terminateServer(Keybinding *keybinding) {
    wl_display_terminate(server.wlDisplay);
}

// action
static void switchVT(Keybinding *keybinding) {
    if (server.session) {
        unsigned vt = keybinding->sym - XKB_KEY_XF86Switch_VT_1 + 1;
        wlr_session_change_vt(server.session, vt);
    }
}

static KeybindingModifiersNode *keybindingModifiersNodeCreate(KeybindingManager *manager, uint32_t modifiers) {
    KeybindingModifiersNode *node = calloc(1, sizeof(KeybindingModifiersNode));
    if (!node) {
        wlr_log(WLR_ERROR, "Unable to allocate KeybindingModifiersNode");
        return NULL;
    }

    node->manager   = manager;
    node->modifiers = modifiers;
    wl_list_init(&node->keybindings);
    wl_list_insert(&manager->modifiersNodes, &node->link);

    return node;
}

static Keybinding *keybindingCreate(KeybindingModifiersNode *node, uint32_t modifiers,
                                    xkb_keysym_t sym, KeybindingAction action) {
    Keybinding *keybinding = calloc(1, sizeof(Keybinding));
    if (!keybinding) {
        wlr_log(WLR_ERROR, "Unable to allocate Keybinding");
        return NULL;
    }

    keybinding->modifiers     = modifiers;
    keybinding->sym           = sym;
    keybinding->action        = action;
    keybinding->modifiersNode = node;
    wl_list_insert(&node->keybindings, &keybinding->link);

    return keybinding;
}

void keybindingManagerDestroy(KeybindingManager *manager) {
    if (!manager) {
        return;
    }

    KeybindingModifiersNode *node, *nextNode;
    wl_list_for_each_safe(node, nextNode, &manager->modifiersNodes, link) {
        Keybinding *keybinding, *nextKeybinding;
        wl_list_for_each_safe(keybinding, nextKeybinding, &node->keybindings, link) {
            wl_list_remove(&keybinding->link);
            free(keybinding);
        }
        wl_list_remove(&node->link);
        free(node);
    }

    free(manager);
}

KeybindingManager *keybindingManagerCreate() {
    KeybindingManager *manager = calloc(1, sizeof(KeybindingManager));
    if (!manager) {
        wlr_log(WLR_ERROR, "Unable to allocate KeybindingManager");
        return NULL;
    }

    wl_list_init(&manager->modifiersNodes);

    // logo
    KeybindingModifiersNode *logo = keybindingModifiersNodeCreate(manager, WLR_MODIFIER_LOGO);
    if (!logo) {
        wlr_log(WLR_ERROR, "Unable to create KeybindingModifiersNode: logo");
        keybindingManagerDestroy(manager);
        return NULL;
    }
    keybindingCreate(logo, logo->modifiers, XKB_KEY_d, openLauncher);
    keybindingCreate(logo, logo->modifiers, XKB_KEY_Return, openTerminal);
    keybindingCreate(logo, logo->modifiers, XKB_KEY_q, closeFocusedView);
    keybindingCreate(logo, logo->modifiers, XKB_KEY_Tab, cycleView);

    // ctrl+alt
    KeybindingModifiersNode *ctrlAlt =
            keybindingModifiersNodeCreate(manager, WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT);
    if (!ctrlAlt) {
        wlr_log(WLR_ERROR, "Unable to create KeybindingModifiersNode: ctrlAlt");
        keybindingManagerDestroy(manager);
        return NULL;
    }
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_Escape, terminateServer);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_1, switchVT);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_2, switchVT);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_3, switchVT);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_4, switchVT);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_5, switchVT);
    keybindingCreate(ctrlAlt, ctrlAlt->modifiers, XKB_KEY_XF86Switch_VT_6, switchVT);

    return manager;
}