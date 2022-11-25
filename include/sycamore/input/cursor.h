#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include <stdbool.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>

struct sycamore_output;
struct sycamore_seat;

enum cursor_image_mode {
    HIDDEN,
    IMAGE,
    SURFACE,
};

struct sycamore_cursor {
    struct wlr_cursor *wlr_cursor;
    struct wlr_xcursor_manager *xcursor_manager;
    struct wlr_pointer_gestures_v1 *gestures;

    enum cursor_image_mode image_mode;
    const char *image; // image name for IMAGE mode

    size_t pressed_button_count;

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

    struct sycamore_seat *seat;
};

void cursor_set_hidden(struct sycamore_cursor *cursor);

/**
 * @brief Set cursor image
 *
 * @note This will avoid setting duplicate image. If image is NULL, hide cursor.
 */
void cursor_set_image(struct sycamore_cursor *cursor, const char *image);

/**
 * @brief Set cursor image surface
 *
 * @note If surface is NULL, hide cursor.
 */
void cursor_set_image_surface(struct sycamore_cursor *cursor,
        struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y);

void cursor_warp(struct sycamore_cursor *cursor, double lx, double ly);

/**
 * @brief Warp cursor again and refresh image
 */
void cursor_refresh(struct sycamore_cursor *cursor);

struct sycamore_output *cursor_at_output(struct sycamore_cursor *cursor,
        struct wlr_output_layout *layout);

struct sycamore_cursor *sycamore_cursor_create(struct sycamore_seat *seat,
        struct wl_display *display, struct wlr_output_layout *output_layout);

void sycamore_cursor_destroy(struct sycamore_cursor *cursor);

#endif //SYCAMORE_CURSOR_H