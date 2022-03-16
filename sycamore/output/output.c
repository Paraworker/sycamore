#include "sycamore/output/output.h"
#include <stdlib.h>
#include <wlr/util/log.h>

static void handle_output_frame(struct wl_listener *listener, void *data) {
    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct sycamore_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;

    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
            scene, output->wlr_output);

    /* Render the scene if needed and commit the output */
    wlr_scene_output_commit(scene_output);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void handle_output_destroy(struct wl_listener *listener, void *data) {
    struct sycamore_output* output = wl_container_of(listener, output, destroy);
    output->wlr_output = NULL;
    sycamore_output_destroy(output);
}

struct sycamore_output* sycamore_output_create(struct sycamore_server* server,
        struct wlr_output *wlr_output) {
    struct sycamore_output *output = calloc(1, sizeof(struct sycamore_output));
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to allocate sycamore output");
        return NULL;
    }

    /* Sets up a listener for the frame notify event. */
    output->frame.notify = handle_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    output->wlr_output = wlr_output;
    output->server = server;

    return output;
}

void sycamore_output_destroy(struct sycamore_output* output) {
    if (!output) {
        return;
    }

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);

    if (output->wlr_output) {
        wlr_output_destroy(output->wlr_output);
    }

    free(output);
}

void handle_backend_new_output(struct wl_listener *listener, void *data) {
    struct sycamore_server *server =
            wl_container_of(listener, server, backend_new_output);
    struct wlr_output *wlr_output = data;

    /* Configures the output created by the backend to use our allocator
     * and our renderer. Must be done once, before commiting the output */
    if (!wlr_output_init_render(wlr_output, server->allocator,
                                server->renderer)) {
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

    /* Allocates and configures our state for this output */
    struct sycamore_output *output = sycamore_output_create(server, wlr_output);
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to create sycamore_output");
        return;
    }

    wl_list_insert(&server->all_outputs, &output->link);

    /* Adds this to the output layout. The add_auto function arranges all_outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of all_outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

