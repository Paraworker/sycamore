#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/view.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"
#include "sycamore/util/box.h"

void viewInit(View *view, ViewType type, struct wlr_surface *surface, const ViewInterface *interface) {
    view->sceneDesc  = SCENE_DESC_VIEW;
    view->interface  = interface;
    view->wlrSurface = surface;
    view->type       = type;

    view->mapped       = false;
    view->isFullscreen = false;
    view->isMaximized  = false;

    wl_list_init(&view->ptrs);
}

void viewMap(View *view, struct wlr_output *fullscreenOutput, bool maximized, bool fullscreen) {
    if (view->mapped) {
        return;
    }

    Output *output = cursorAtOutput(server.seat->cursor);

    viewSetToOutputCenter(view, output);

    viewSetMaximized(view, output, maximized);

    /* If there is a request to be opened fullscreen
     * on a specific output, try to satisfy it. */
    if (fullscreenOutput && fullscreenOutput->data) {
        output = fullscreenOutput->data;
    }

    viewSetFullscreen(view, output, fullscreen);

    view->interface->map(view);

    wl_list_insert(&server.mappedViews, &view->link);
    view->mapped = true;

    viewSetFocus(view);

    cursorRebase(server.seat->cursor);
}

void viewUnmap(View *view) {
    if (!view->mapped) {
        return;
    }

    wl_list_remove(&view->link);

    ViewPtr *ptr, *next;
    wl_list_for_each_safe(ptr, next, &view->ptrs, link) {
        viewPtrDisconnect(ptr);
    }

    view->interface->unmap(view);

    view->mapped = false;

    cursorRebase(server.seat->cursor);
}

void viewDestroy(View *view) {
    if (!view || view->mapped) {
        return;
    }

    view->interface->destroy(view);
}

Output *viewGetOutput(View *view) {
    struct wlr_surface *surface = view->wlrSurface;

    struct wl_list *surfaceOutputs = &surface->current_outputs;
    if (wl_list_empty(surfaceOutputs)) {
        return NULL;
    }

    struct wlr_surface_output *surfaceOutput =
            wl_container_of(surface->current_outputs.prev, surfaceOutput, link);
    return surfaceOutput->output->data;
}

void viewSetFocus(View *view) {
    /* Note: this function only deals with keyboard focus. */
    if (!view || view->type == VIEW_TYPE_UNKNOWN) {
        return;
    }

    View *prevView = server.focusedView.view;
    if (prevView == view) {
        /* Don't refocus */
        return;
    }

    if (prevView) {
        /* Deactivate the previously focused view. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret. */
        prevView->interface->setActivated(prevView, false);
        viewPtrDisconnect(&server.focusedView);
    }

    /* Move the view to the front */
    wlr_scene_node_raise_to_top(&view->sceneTree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server.mappedViews, &view->link);

    /* Activate the new view */
    view->interface->setActivated(view, true);

    if (!server.focusedLayer) {
        seatSetKeyboardFocus(server.seat, view->wlrSurface);
    }

    viewPtrConnect(&server.focusedView, view);
}

void viewSetFullscreen(View *view, Output *output, bool fullscreen) {
    if (fullscreen == view->isFullscreen) {
        return;
    }

    if (!fullscreen) {
        // Restore from fullscreen mode
        wlr_scene_node_set_enabled(&server.scene->shell.top->node, true);

        struct wlr_box *restore = &view->restore.fullscreen;
        view->interface->setSize(view, restore->width, restore->height);
        VIEW_MOVE_TO(view, restore->x, restore->y);
        view->isFullscreen = fullscreen;
        view->interface->setFullscreen(view, fullscreen);
        return;
    }

    // Set to fullscreen mode
    if (!output) {
        return;
    }

    struct wlr_box fullBox;
    wlr_output_layout_get_box(server.outputLayout,
                              output->wlrOutput, &fullBox);

    view->restore.fullscreen.x = VIEW_X(view);
    view->restore.fullscreen.y = VIEW_Y(view);
    struct wlr_box windowBox;
    view->interface->getGeometry(view, &windowBox);
    view->restore.fullscreen.width = windowBox.width;
    view->restore.fullscreen.height = windowBox.height;

    wlr_scene_node_set_enabled(&server.scene->shell.top->node, false);

    VIEW_MOVE_TO(view, fullBox.x, fullBox.y);
    view->interface->setSize(view, fullBox.width, fullBox.height);
    view->isFullscreen = fullscreen;
    view->interface->setFullscreen(view, fullscreen);
}

void viewSetMaximized(View *view, Output *output, bool maximized) {
    if (maximized == view->isMaximized) {
        return;
    }

    if (!maximized) {
        /* Restore from maximized mode */
        struct wlr_box *restore = &view->restore.maximize;

        view->interface->setMaximized(view, maximized);
        view->interface->setSize(view, restore->width, restore->height);
        VIEW_MOVE_TO(view, restore->x, restore->y);
        view->isMaximized = maximized;
        return;
    }

    /* Set to maximized mode */
    if (!output) {
        return;
    }

    struct wlr_box maxBox = output->usableArea;

    view->restore.maximize.x = VIEW_X(view);
    view->restore.maximize.y = VIEW_Y(view);
    struct wlr_box windowBox;
    view->interface->getGeometry(view, &windowBox);
    view->restore.maximize.width = windowBox.width;
    view->restore.maximize.height = windowBox.height;

    VIEW_MOVE_TO(view, maxBox.x, maxBox.y);
    view->interface->setSize(view, maxBox.width, maxBox.height);
    view->isMaximized = maximized;
    view->interface->setMaximized(view, maximized);
}

void viewSetToOutputCenter(View *view, Output *output) {
    if (!output) {
        return;
    }

    struct wlr_box outputBox;
    wlr_output_layout_get_box(server.outputLayout, output->wlrOutput, &outputBox);
    if (wlr_box_empty(&outputBox)) {
        wlr_log(WLR_ERROR, "outputBox is empty.");
        return;
    }

    int32_t centerX = 0, centerY = 0;
    boxGetCenterCoords(&outputBox, &centerX, &centerY);

    struct wlr_box viewBox;
    view->interface->getGeometry(view, &viewBox);

    viewBox.x = centerX - (viewBox.width / 2);
    viewBox.y = centerY - (viewBox.height / 2);

    // Don't allow view's top being out of output.
    if (viewBox.y < outputBox.y) {
        viewBox.y = outputBox.y;
    }

    VIEW_MOVE_TO(view, viewBox.x, viewBox.y);
}

void viewPtrConnect(ViewPtr *ptr, View *view) {
    ptr->view = view;
    wl_list_insert(&view->ptrs, &ptr->link);
}

void viewPtrDisconnect(ViewPtr *ptr) {
    ptr->view = NULL;
    wl_list_remove(&ptr->link);
}