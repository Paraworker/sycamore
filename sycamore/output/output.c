#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>
#include "sycamore/desktop/shell/layer_shell/layer.h"
#include "sycamore/input/cursor.h"
#include "sycamore/input/seat.h"
#include "sycamore/output/output.h"
#include "sycamore/server.h"
#include "sycamore/util/box.h"

static void onOutputFrame(struct wl_listener *listener, void *data) {
    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    Output *output = wl_container_of(listener, output, frame);

    struct wlr_scene_output *sceneOutput =
            wlr_scene_get_scene_output(server.scene->wlrScene, output->wlrOutput);

    /* Render the scene if needed and commit the output */
    wlr_scene_output_commit(sceneOutput);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(sceneOutput, &now);
}

static void onOutputRequestState(struct wl_listener *listener, void *data) {
    Output *output = wl_container_of(listener, output, requestState);
    const struct wlr_output_event_request_state *event = data;

    wlr_output_commit_state(output->wlrOutput, event->state);
}

static void onOutputDestroy(struct wl_listener *listener, void *data) {
    Output *output = wl_container_of(listener, output, destroy);

    output->wlrOutput = NULL;

    outputDestroy(output);
}

Output *outputCreate(struct wlr_output *wlrOutput) {
    Output *output = calloc(1, sizeof(Output));
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to allocate Output");
        return NULL;
    }

    output->wlrOutput = wlrOutput;

    for (int i = 0; i < LAYERS_ALL; ++i) {
        wl_list_init(&output->layers[i]);
    }

    output->frame.notify = onOutputFrame;
    wl_signal_add(&wlrOutput->events.frame, &output->frame);
    output->requestState.notify = onOutputRequestState;
    wl_signal_add(&wlrOutput->events.request_state, &output->requestState);
    output->destroy.notify = onOutputDestroy;
    wl_signal_add(&wlrOutput->events.destroy, &output->destroy);

    return output;
}

struct wlr_output_mode *outputMaxMode(struct wlr_output *output) {
    if (!output || wl_list_empty(&output->modes)) {
        return NULL;
    }

    uint32_t maxRefresh = 0, maxResolution = 0;
    struct wlr_output_mode *mode, *maxMode;
    wl_list_for_each(mode, &output->modes, link) {
        int32_t resolution = mode->width * mode->height;
        if (resolution > maxResolution) {
            maxResolution = resolution;
            maxRefresh = mode->refresh;
            maxMode = mode;
        } else if (resolution == maxResolution && mode->refresh > maxRefresh) {
            maxRefresh = mode->refresh;
            maxMode = mode;
        }
    }

    return maxMode;
}

void outputEnsureCursor(Output *output, Cursor *cursor) {
    if (!cursor) {
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

    cursorWarp(cursor, centerX, centerY);
    seatopPointerRebase(cursor->seat);
}

static void outputSetupCursor(Output *output, Cursor *cursor) {
    /* Setup cursor for a new output */
    wlr_xcursor_manager_load(cursor->xcursorManager, output->wlrOutput->scale);

    if (wl_list_length(&server.allOutputs) == 1) {
        // If this is the only output, center cursor.
        outputEnsureCursor(output, cursor);
    } else {
        cursorRefresh(cursor);
    }
}

void outputDestroy(Output *output) {
    if (!output) {
        return;
    }

    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->requestState.link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->link);

    for (size_t i = 0; i < LAYERS_ALL; ++i) {
        Layer *layer, *next;
        wl_list_for_each_safe(layer, next, &output->layers[i], link) {
            wl_list_remove(&layer->link);
            layer->linked = false;
        }
    }

    if (output->wlrOutput) {
        wlr_output_destroy(output->wlrOutput);
    }

    free(output);
}

void onBackendNewOutput(struct wl_listener *listener, void *data) {
    struct wlr_output *wlrOutput = data;
    wlr_log(WLR_DEBUG, "new output: %s", wlrOutput->name);

    /* Configures the output created by the backend to use our allocator
     * and our renderer. Must be done once, before commiting the output */
    if (!wlr_output_init_render(wlrOutput, server.allocator, server.renderer)) {
        wlr_log(WLR_ERROR, "Unable to init output render");
        return;
    }

    /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
     * before we can use the output. The mode is a tuple of (width, height,
     * refresh rate), and each monitor supports only a specific set of modes. We
     * just pick the monitor's preferred mode, a more sophisticated compositor
     * would let the user configure it. */
    if (!wl_list_empty(&wlrOutput->modes)) {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlrOutput);
        wlr_output_set_mode(wlrOutput, mode);
        wlr_output_enable(wlrOutput, true);
        if (!wlr_output_commit(wlrOutput)) {
            return;
        }
    }

    Output *output = outputCreate(wlrOutput);
    if (!output) {
        wlr_log(WLR_ERROR, "Unable to create Output");
        return;
    }

    wlrOutput->data = output;

    wlr_output_layout_add_auto(server.outputLayout, wlrOutput);

    wlr_output_layout_get_box(server.outputLayout, wlrOutput, &output->usableArea);
    wl_list_insert(&server.allOutputs, &output->link);

    outputSetupCursor(output, server.seat->cursor);
}
