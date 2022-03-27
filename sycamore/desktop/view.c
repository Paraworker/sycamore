#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

#include "sycamore/desktop/view.h"
#include "sycamore/desktop/scene.h"

void view_map(struct sycamore_view* view, struct wlr_output* output, bool maximized, bool fullscreen) {
    view_set_maximized(view, maximized);
    view_set_fullscreen(view, output, fullscreen);
    wl_list_insert(&view->server->mapped_views, &view->link);
    focus_view(view);
}

void view_unmap(struct sycamore_view* view) {
    wl_list_remove(&view->link);
    if (view->server->seat->cursor->grabbed_view == view) {
        view->server->seat->cursor->grabbed_view = NULL;
    }
    if (view->server->desktop_focused_view == view) {
        view->server->desktop_focused_view = NULL;
    }

    double sx, sy;
    struct sycamore_cursor* cursor = view->server->seat->cursor;
    struct wlr_surface* surface = desktop_surface_at(view->server->scene,
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, &sx, &sy);
    update_pointer_focus(cursor, surface, sx, sy);
}

void focus_view(struct sycamore_view *view) {
    /* Note: this function only deals with keyboard focus. */
    if (view == NULL || view->view_type == VIEW_TYPE_UNKNOWN) {
        return;
    }
    struct sycamore_server *server = view->server;
    struct sycamore_view *prev_view = server->desktop_focused_view;
    if (prev_view == view) {
        /* Don't re-focus an already focused view. */
        return;
    }

    if (prev_view) {
        /*
         * Deactivate the previously focused view. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret.
         */
        prev_view->interface->set_activated(prev_view, false);
    }

    /* Move the view to the front */
    wlr_scene_node_raise_to_top(view->scene_node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->mapped_views, &view->link);

    /* Activate the new view */
    view->interface->set_activated(view, true);

    /* Tell the seat to have the keyboard enter this surface. wlroots will keep
     * track of this and automatically send key events to the appropriate
     * clients without additional work on your part. */
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat->wlr_seat);

    if (keyboard) {
        wlr_seat_keyboard_notify_enter(server->seat->wlr_seat, view->interface->get_wlr_surface(view),
                                       keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    } else {
        wlr_seat_keyboard_notify_clear_focus(server->seat->wlr_seat);
    }

    server->desktop_focused_view = view;
}

void view_set_fullscreen(struct sycamore_view* view, struct wlr_output* output, bool fullscreen) {
    if (fullscreen == view->is_fullscreen) {
        return;
    }

    view->is_fullscreen = fullscreen;
    view->interface->set_fullscreen(view, fullscreen);

    if (fullscreen) {
        view->fullscreen_restore.x = view->x;
        view->fullscreen_restore.y = view->y;

        struct wlr_box window_box;
        view->interface->get_geometry(view, &window_box);
        view->fullscreen_restore.width = window_box.width;
        view->fullscreen_restore.height = window_box.height;

        struct wlr_box fullscreen_box;
        wlr_output_layout_get_box(view->server->output_layout,
                                  output, &fullscreen_box);
        view->x = fullscreen_box.x;
        view->y = fullscreen_box.y;

        wlr_scene_node_place_above(&view->server->scene->trees.shell_view->node,
                                   &view->server->scene->trees.shell_top->node);
        wlr_scene_node_set_position(view->scene_node , fullscreen_box.x, fullscreen_box.y);
        view->interface->set_size(view, fullscreen_box.width, fullscreen_box.height);
    } else {
        /* Restore from fullscreen mode */
        view->x = view->fullscreen_restore.x;
        view->y = view->fullscreen_restore.y;

        wlr_scene_node_place_below(&view->server->scene->trees.shell_view->node,
                                   &view->server->scene->trees.shell_top->node);
        view->interface->set_size(view, view->fullscreen_restore.width, view->fullscreen_restore.height);
        wlr_scene_node_set_position(view->scene_node , view->fullscreen_restore.x, view->fullscreen_restore.y);
    }
}

void view_set_maximized(struct sycamore_view* view, bool maximized) {
    if (maximized == view->is_maximized) {
        return;
    }

    view->is_maximized = maximized;
    view->interface->set_maximized(view, maximized);

    if (maximized) {
        view->maximize_restore.x = view->x;
        view->maximize_restore.y = view->y;

        struct wlr_box window_box;
        view->interface->get_geometry(view, &window_box);
        view->maximize_restore.width = window_box.width;
        view->maximize_restore.height = window_box.height;

        struct wlr_box max_box;
        wlr_output_layout_get_box(view->server->output_layout,
                                  NULL, &max_box);
        view->x = max_box.x;
        view->y = max_box.y;

        wlr_scene_node_set_position(view->scene_node , max_box.x, max_box.y);
        view->interface->set_size(view, max_box.width, max_box.height);
    } else {
        /* Restore from maximized mode */
        view->x = view->maximize_restore.x;
        view->y = view->maximize_restore.y;

        view->interface->set_size(view, view->maximize_restore.width, view->maximize_restore.height);
        wlr_scene_node_set_position(view->scene_node , view->maximize_restore.x, view->maximize_restore.y);
    }
}

