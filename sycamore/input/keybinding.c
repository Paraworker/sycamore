#include "sycamore/input/keybinding.h"
#include "sycamore/desktop/view.h"
#include <stdlib.h>
#include <wlr/util/log.h>

bool handle_keybinding(struct sycamore_keybinding_manager* manager, uint32_t modifiers, xkb_keysym_t sym) {
    if (modifiers == 0) {
        return false;
    }

    struct sycamore_modifiers_node* node;
    wl_list_for_each(node, &manager->modifiers_nodes, link) {
        if (modifiers == node->modifiers) {
            struct sycamore_keybinding* keybinding;
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
static void cycle_view(struct sycamore_server *server, struct sycamore_keybinding* keybinding) {
    /* Cycle to the next view */
    if (wl_list_length(&server->mapped_views) < 2) {
        return;
    }

    struct sycamore_view *next_view = wl_container_of(
            server->mapped_views.prev, next_view, link);
    focus_view(next_view);

    double sx, sy;
    struct sycamore_cursor* cursor = server->seat->cursor;
    struct wlr_surface* surface = desktop_surface_at(server,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);
    update_pointer_focus(cursor, surface, sx, sy);
}

/* action */
static void terminate_server(struct sycamore_server *server, struct sycamore_keybinding* keybinding) {
    wl_display_terminate(server->wl_display);
}

static struct sycamore_modifiers_node* sycamore_modifiers_node_create(struct sycamore_keybinding_manager* manager, uint32_t modifiers) {
    struct sycamore_modifiers_node* node = calloc(1, sizeof(struct sycamore_modifiers_node));
    if (!node) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_modifiers_node");
        return NULL;
    }

    node->manager = manager;
    node->modifiers = modifiers;
    wl_list_init(&node->keybindings);
    wl_list_insert(&manager->modifiers_nodes, &node->link);

    return node;
}

static struct sycamore_keybinding * sycamore_keybinding_create(struct sycamore_modifiers_node *node,
        uint32_t modifiers, xkb_keysym_t sym,
        void (*action)(struct sycamore_server *server, struct sycamore_keybinding* keybinding)) {
    struct sycamore_keybinding* keybinding = calloc(1, sizeof(struct sycamore_keybinding));
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

void sycamore_keybinding_manager_destroy(struct sycamore_keybinding_manager* manager) {
    if (!manager) {
        return;
    }

    struct sycamore_modifiers_node *node, *next_node;
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

struct sycamore_keybinding_manager* sycamore_keybinding_manager_create(struct sycamore_server* server) {
    struct sycamore_keybinding_manager* manager =
            calloc(1, sizeof(struct sycamore_keybinding_manager));
    if (!manager) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_keybinding_manager");
        return NULL;
    }

    wl_list_init(&manager->modifiers_nodes);
    manager->server = server;

    /* alt */
    struct sycamore_modifiers_node* alt = sycamore_modifiers_node_create(manager, WLR_MODIFIER_ALT);
    if (!alt) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_modifiers_node");
        sycamore_keybinding_manager_destroy(manager);
        return NULL;
    }
    sycamore_keybinding_create(alt, alt->modifiers, XKB_KEY_Tab, cycle_view);

    /* ctrl+alt */
    struct sycamore_modifiers_node* ctrl_alt =
            sycamore_modifiers_node_create(manager, WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT);
    if (!ctrl_alt) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_modifiers_node");
        sycamore_keybinding_manager_destroy(manager);
        return NULL;
    }
    sycamore_keybinding_create(ctrl_alt, ctrl_alt->modifiers, XKB_KEY_Escape, terminate_server);

    return manager;
}







