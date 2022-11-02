#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/layer.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"
#include "sycamore/util/box.h"

static void handle_output_frame(struct wl_listener *listener, void *data) {
    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct sycamore_output *output = wl_container_of(listener, output, frame);

    struct wlr_scene_output *scene_output =
            wlr_scene_get_scene_output(server.scene->wlr_scene, output->wlr_output);

    /* Render the scene if needed and commit the output */
    wlr_scene_output_commit(scene_output);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void handle_output_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_output *output = wl_container_of(listener, output, destroy);

    output->wlr_output = NULL;

    sycamore_output_destroy(output);
}

struct sycamore_output *sycamore_output_create(struct wlr_output *wlr_output) {
    struct sycamore_output *output =
            calloc(1, sizeof(struct sycamore_output));
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore_output");
        return NULL;
    }

    output->wlr_output = wlr_output;

    for (int i = 0; i < LAYERS_ALL; ++i) {
        wl_list_init(&output->layers[i]);
    }

    output->frame.notify = handle_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    return output;
}

struct wlr_output_mode *output_max_mode(struct wlr_output *output) {
    if (!output || wl_list_empty(&output->modes)) {
        return NULL;
    }

    int32_t max_refresh = 0, max_resolution = 0;
    struct wlr_output_mode *mode, *max_mode;
    wl_list_for_each(mode, &output->modes, link) {
        int32_t resolution = mode->width * mode->height;
        if (resolution > max_resolution) {
            max_resolution = resolution;
            max_refresh = mode->refresh;
            max_mode = mode;
        } else if (resolution == max_resolution && mode->refresh > max_refresh) {
            max_refresh = mode->refresh;
            max_mode = mode;
        }
    }

    return max_mode;
}

void output_ensure_cursor(struct sycamore_output *output, struct sycamore_cursor *cursor) {
    if (!cursor) {
        return;
    }

    struct wlr_box output_box;
    wlr_output_layout_get_box(server.output_layout, output->wlr_output, &output_box);
    if (wlr_box_empty(&output_box)) {
        wlr_log(WLR_ERROR, "output_box is empty.");
        return;
    }

    int32_t center_x = 0, center_y = 0;
    box_get_center_coords(&output_box, &center_x, &center_y);

    cursor_warp(cursor, center_x, center_y);
    seatop_pointer_rebase(cursor->seat);
}

static void output_setup_cursor(struct sycamore_output *output, struct sycamore_cursor *cursor) {
    /* Setup cursor for a new output */
    wlr_xcursor_manager_load(cursor->xcursor_manager, output->wlr_output->scale);

    if (wl_list_length(&server.all_outputs) == 1) {
        // If this is the only output, center cursor.
        output_ensure_cursor(output, cursor);
    } else {
        cursor_refresh(cursor);
    }
}

void sycamore_output_destroy(struct sycamore_output *output) {
    if (!output) {
        return;
    }

    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->link);

    for (int i = 0; i < LAYERS_ALL; ++i) {
        struct sycamore_layer *layer, *next;
        wl_list_for_each_safe(layer, next, &output->layers[i], link) {
            wl_list_remove(&layer->link);
            layer->linked = false;
        }
    }

    if (output->wlr_output) {
        wlr_output_destroy(output->wlr_output);
    }

    free(output);
}

void handle_backend_new_output(struct wl_listener *listener, void *data) {
    struct wlr_output *wlr_output = data;
    wlr_log(WLR_DEBUG, "new output: %s", wlr_output->name);

    /* Configures the output created by the backend to use our allocator
     * and our renderer. Must be done once, before commiting the output */
    if (!wlr_output_init_render(wlr_output, server.allocator,
                                server.renderer)) {
        wlr_log(WLR_ERROR, "Unable to init output render");
        return;
    }

    /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
     * before we can use the output. The mode is a tuple of (width, height,
     * refresh rate), and each monitor supports only a specific set of modes. We
     * just pick the monitor's preferred mode, a more sophisticated compositor
     * would let the user configure it. */
    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        if (!wlr_output_commit(wlr_output)) {
            return;
        }
    }

    struct sycamore_output *output = sycamore_output_create(wlr_output);
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_output");
        return;
    }

    wlr_output->data = output;

    wlr_output_layout_add_auto(server.output_layout, wlr_output);

    wlr_output_layout_get_box(server.output_layout, wlr_output, &output->usable_area);
    wl_list_insert(&server.all_outputs, &output->link);

    output_setup_cursor(output, server.seat->cursor);
}
