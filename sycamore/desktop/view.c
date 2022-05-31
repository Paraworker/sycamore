#include <stdbool.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"

void view_init(struct sycamore_view *view, struct wlr_surface *surface,
        const struct view_interface *interface, struct sycamore_server *server) {
    view->scene_descriptor = SCENE_DESC_VIEW;
    view->interface = interface;
    view->wlr_surface = surface;
    view->view_type = VIEW_TYPE_XDG_SHELL;

    view->mapped = false;
    view->is_fullscreen = false;
    view->is_maximized = false;

    wl_list_init(&view->ptrs);

    view->server = server;
}

void view_map(struct sycamore_view *view,
        struct wlr_output *fullscreen_output, bool maximized, bool fullscreen) {
    if (view->mapped) {
        return;
    }

    wl_list_insert(&view->server->mapped_views, &view->link);

    struct sycamore_server *server = view->server;
    struct wlr_output_layout *layout = server->output_layout;
    struct wlr_output *output = cursor_at_output(server->seat->cursor, layout);
    struct wlr_box box;
    wlr_output_layout_get_box(layout, output, &box);

    view->x = box.x;
    view->y = box.y;
    wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);

    if (maximized) {
        if (output) {
            struct sycamore_output *sycamore_output = output->data;
            view_set_maximized(view, &sycamore_output->usable_area, maximized);
        } else {
            view_set_maximized(view, &box, maximized);
        }
    }

    if (fullscreen) {
        if (fullscreen_output) {
            struct wlr_box full_box;
            wlr_output_layout_get_box(layout, output, &full_box);
            view_set_fullscreen(view, &full_box, fullscreen);
        } else {
            view_set_fullscreen(view, &box, fullscreen);
        }
    }

    view->interface->map(view);

    view->mapped = true;

    view_set_focus(view);

    struct sycamore_seat *seat = view->server->seat;
    seat->seatop_impl->cursor_rebase(seat);
}

void view_unmap(struct sycamore_view *view) {
    if (!view->mapped) {
        return;
    }

    wl_list_remove(&view->link);

    struct view_ptr *ptr, *next;
    wl_list_for_each_safe(ptr, next, &view->ptrs, link) {
        view_ptr_disconnect(ptr);
    }

    view->interface->unmap(view);

    view->mapped = false;

    struct sycamore_seat *seat = view->server->seat;
    seat->seatop_impl->cursor_rebase(seat);
}

void view_destroy(struct sycamore_view *view) {
    if (!view || view->mapped) {
        return;
    }

    view->interface->destroy(view);
}

struct sycamore_output *view_get_main_output(struct sycamore_view *view) {
    struct wlr_surface *surface = view->wlr_surface;

    struct wl_list *surface_outputs = &surface->current_outputs;
    if (wl_list_empty(surface_outputs)) {
        return NULL;
    }

    struct wlr_surface_output *surface_output = wl_container_of(surface->current_outputs.prev, surface_output, link);
    return surface_output->output->data;
}

void view_set_focus(struct sycamore_view *view) {
    /* Note: this function only deals with keyboard focus. */
    if (!view || view->view_type == VIEW_TYPE_UNKNOWN) {
        return;
    }
    struct sycamore_server *server = view->server;
    struct sycamore_seat *seat = server->seat;
    struct sycamore_view *prev_view = server->focused_view.view;
    if (prev_view == view) {
        /* Don't refocus */
        return;
    }

    if (prev_view) {
        /* Deactivate the previously focused view. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret. */
        prev_view->interface->set_activated(prev_view, false);
        view_ptr_disconnect(&server->focused_view);
    }

    /* Move the view to the front */
    wlr_scene_node_raise_to_top(&view->scene_tree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->mapped_views, &view->link);

    /* Activate the new view */
    view->interface->set_activated(view, true);

    if (!seat->focused_layer) {
        seat_set_keyboard_focus(seat, view->wlr_surface);
    }

    view_ptr_connect(&server->focused_view, view);
}

void view_set_fullscreen(struct sycamore_view *view,
        const struct wlr_box *full_box, bool fullscreen) {
    if (fullscreen == view->is_fullscreen) {
        return;
    }

    if (fullscreen) {
        if (!full_box) {
            return;
        }

        view->fullscreen_restore.x = view->x;
        view->fullscreen_restore.y = view->y;
        struct wlr_box window_box;
        view->interface->get_geometry(view, &window_box);
        view->fullscreen_restore.width = window_box.width;
        view->fullscreen_restore.height = window_box.height;

        view->x = full_box->x;
        view->y = full_box->y;

        wlr_scene_node_place_above(&view->server->scene->trees.shell_view->node,
                                   &view->server->scene->trees.shell_top->node);
        wlr_scene_node_set_position(&view->scene_tree->node , full_box->x, full_box->y);
        view->interface->set_size(view, full_box->width, full_box->height);
    } else {
        /* Restore from fullscreen mode */
        view->x = view->fullscreen_restore.x;
        view->y = view->fullscreen_restore.y;

        wlr_scene_node_place_below(&view->server->scene->trees.shell_view->node,
                                   &view->server->scene->trees.shell_top->node);
        view->interface->set_size(view, view->fullscreen_restore.width, view->fullscreen_restore.height);
        wlr_scene_node_set_position(&view->scene_tree->node , view->fullscreen_restore.x, view->fullscreen_restore.y);
    }

    view->is_fullscreen = fullscreen;
    view->interface->set_fullscreen(view, fullscreen);
}

void view_set_maximized(struct sycamore_view *view,
        const struct wlr_box *max_box, bool maximized) {
    if (maximized == view->is_maximized) {
        return;
    }

    if (maximized) {
        if (!max_box) {
            return;
        }

        view->maximize_restore.x = view->x;
        view->maximize_restore.y = view->y;
        struct wlr_box window_box;
        view->interface->get_geometry(view, &window_box);
        view->maximize_restore.width = window_box.width;
        view->maximize_restore.height = window_box.height;

        view->x = max_box->x;
        view->y = max_box->y;

        wlr_scene_node_set_position(&view->scene_tree->node , max_box->x, max_box->y);
        view->interface->set_size(view, max_box->width, max_box->height);
    } else {
        /* Restore from maximized mode */
        view->x = view->maximize_restore.x;
        view->y = view->maximize_restore.y;

        view->interface->set_size(view, view->maximize_restore.width, view->maximize_restore.height);
        wlr_scene_node_set_position(&view->scene_tree->node , view->maximize_restore.x, view->maximize_restore.y);
    }

    view->is_maximized = maximized;
    view->interface->set_maximized(view, maximized);
}

void view_ptr_connect(struct view_ptr *ptr, struct sycamore_view *view) {
    ptr->view = view;
    wl_list_insert(&view->ptrs, &ptr->link);
}

void view_ptr_disconnect(struct view_ptr *ptr) {
    ptr->view = NULL;
    wl_list_remove(&ptr->link);
}

