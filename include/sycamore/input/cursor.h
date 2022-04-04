#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include "sycamore/server.h"
#include "sycamore/input/seat.h"

enum cursor_mode {
    CURSOR_MODE_PASSTHROUGH,
    CURSOR_MODE_MOVE,
    CURSOR_MODE_RESIZE,
};

struct sycamore_cursor {
    struct wlr_cursor *wlr_cursor;
    struct wlr_xcursor_manager *xcursor_manager;
    struct wlr_pointer_gestures_v1 *gestures;
    enum cursor_mode mode;

    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wl_listener swipe_begin;
    struct wl_listener swipe_update;
    struct wl_listener swipe_end;
    struct wl_listener pinch_begin;
    struct wl_listener pinch_update;
    struct wl_listener pinch_end;
    struct wl_listener hold_begin;
    struct wl_listener hold_end;

    /* for cursor move and resize view */
    struct sycamore_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    bool default_setted;
    struct sycamore_seat *seat;
};

void sycamore_cursor_destroy(struct sycamore_cursor *cursor);

void update_pointer_focus(struct sycamore_cursor *cursor,
        struct wlr_surface *surface, double sx, double sy);

void set_interactive(struct sycamore_view *view, enum cursor_mode mode, uint32_t edges);

struct sycamore_cursor *sycamore_cursor_create(struct sycamore_seat *seat,
        struct wlr_output_layout *output_layout);

#endif //SYCAMORE_CURSOR_H
