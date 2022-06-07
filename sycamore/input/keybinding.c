#include <stdlib.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include "sycamore/input/keybinding.h"
#include "sycamore/desktop/view.h"
#include "sycamore/server.h"

bool handle_keybinding(struct sycamore_keybinding_manager *manager, uint32_t modifiers, xkb_keysym_t sym) {
    if (modifiers == 0) {
        return false;
    }

    struct keybinding_modifiers_node *node;
    wl_list_for_each(node, &manager->modifiers_nodes, link) {
        if (modifiers == node->modifiers) {
            struct sycamore_keybinding *keybinding;
            wl_list_for_each(keybinding, &node->keybindings, link) {
                if (sym == keybinding->sym) {
                    keybinding->action(manager->server, keybinding);
                    return true;
                }
            } //for each
        }
    } //for each
    return false;
}

/* action */
static void cycle_view(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    /* Cycle to the next view */
    if (wl_list_length(&server->mapped_views) < 2) {
        return;
    }

    struct sycamore_view *next_view = wl_container_of(
            server->mapped_views.prev, next_view, link);
    view_set_focus(next_view);
    struct sycamore_seat *seat = server->seat;
    seat->seatop_impl->cursor_rebase(seat);
}

/* action */
static void terminate_server(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    wl_display_terminate(server->wl_display);
}

/* action */
static void open_launcher(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "fuzzel -i Papirus ", (void *)NULL);
    }
}

/* action */
static void open_terminal(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "gnome-terminal", (void *)NULL);
    }
}

/* action */
static void open_browser(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "chromium", (void *)NULL);
    }
}

/* action */
static void switch_vt(struct sycamore_server *server, struct sycamore_keybinding *keybinding) {
    struct wlr_session *session = wlr_backend_get_session(server->backend);

    if (session != NULL) {
        unsigned vt = keybinding->sym - XKB_KEY_XF86Switch_VT_1 + 1;
        wlr_session_change_vt(session, vt);
    }
}

static struct keybinding_modifiers_node *keybinding_modifiers_node_create(
        struct sycamore_keybinding_manager *manager, uint32_t modifiers) {
    struct keybinding_modifiers_node *node =
            calloc(1, sizeof(struct keybinding_modifiers_node));
    if (!node) {
        wlr_log(WLR_ERROR, "Unable to allocate keybinding_modifiers_node");
        return NULL;
    }

    node->manager = manager;
    node->modifiers = modifiers;
    wl_list_init(&node->keybindings);
    wl_list_insert(&manager->modifiers_nodes, &node->link);

    return node;
}

static struct sycamore_keybinding *sycamore_keybinding_create(struct keybinding_modifiers_node *node,
        uint32_t modifiers, xkb_keysym_t sym,
        void (*action)(struct sycamore_server *server, struct sycamore_keybinding *keybinding)) {
    struct sycamore_keybinding *keybinding = calloc(1, sizeof(struct sycamore_keybinding));
    if (!keybinding) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_keybinding");
        return NULL;
    }

    keybinding->modifiers = modifiers;
    keybinding->sym = sym;
    keybinding->action = action;
    keybinding->modifiers_node = node;
    wl_list_insert(&node->keybindings, &keybinding->link);

    return keybinding;
}

void sycamore_keybinding_manager_destroy(struct sycamore_keybinding_manager *manager) {
    if (!manager) {
        return;
    }

    struct keybinding_modifiers_node *node, *next_node;
    wl_list_for_each_safe(node, next_node, &manager->modifiers_nodes, link) {
        struct sycamore_keybinding *keybinding, *next_keybinding;
        wl_list_for_each_safe(keybinding, next_keybinding, &node->keybindings, link) {
            wl_list_remove(&keybinding->link);
            free(keybinding);
        }
        wl_list_remove(&node->link);
        free(node);
    }

    free(manager);
}

struct sycamore_keybinding_manager *sycamore_keybinding_manager_create(struct sycamore_server *server) {
    struct sycamore_keybinding_manager *manager =
            calloc(1, sizeof(struct sycamore_keybinding_manager));
    if (!manager) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_keybinding_manager");
        return NULL;
    }

    wl_list_init(&manager->modifiers_nodes);
    manager->server = server;

    /* logo */
    struct keybinding_modifiers_node *logo = keybinding_modifiers_node_create(manager, WLR_MODIFIER_LOGO);
    if (!logo) {
        wlr_log(WLR_ERROR, "Unable to create keybinding_modifiers_node: logo");
        sycamore_keybinding_manager_destroy(manager);
        return NULL;
    }
    sycamore_keybinding_create(logo, logo->modifiers, XKB_KEY_Tab, cycle_view);
    sycamore_keybinding_create(logo, logo->modifiers, XKB_KEY_d, open_launcher);
    sycamore_keybinding_create(logo, logo->modifiers, XKB_KEY_Return, open_terminal);
    sycamore_keybinding_create(logo, logo->modifiers, XKB_KEY_b, open_browser);

    /* ctrl+alt */
    struct keybinding_modifiers_node *ctrl_alt =
            keybinding_modifiers_node_create(manager, WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT);
    if (!ctrl_alt) {
        wlr_log(WLR_ERROR, "Unable to create keybinding_modifiers_node: ctrl_alt");
        sycamore_keybinding_manager_destroy(manager);
        return NULL;
    }
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_Escape, terminate_server);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_1, switch_vt);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_2, switch_vt);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_3, switch_vt);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_4, switch_vt);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_5, switch_vt);
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_XF86Switch_VT_6, switch_vt);

    return manager;
}







