#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include <stdbool.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>

typedef struct Cursor        Cursor;
typedef enum CursorImageMode CursorImageMode;
typedef struct Output        Output;
typedef struct Seat          Seat;

enum CursorImageMode {
    HIDDEN,
    IMAGE,
    SURFACE,
};

struct Cursor {
    struct wlr_cursor              *wlrCursor;
    struct wlr_xcursor_manager     *xcursorManager;
    struct wlr_pointer_gestures_v1 *gestures;

    CursorImageMode imageMode;
    const char      *image; // image name for IMAGE mode

    size_t pressedButtonCount;

    struct wl_listener cursorMotion;
    struct wl_listener cursorMotionAbsolute;
    struct wl_listener cursorButton;
    struct wl_listener cursorAxis;
    struct wl_listener cursorFrame;

    struct wl_listener swipeBegin;
    struct wl_listener swipeUpdate;
    struct wl_listener swipeEnd;
    struct wl_listener pinchBegin;
    struct wl_listener pinchUpdate;
    struct wl_listener pinchEnd;
    struct wl_listener holdBegin;
    struct wl_listener holdEnd;

    Seat *seat;
};

void cursorSetHidden(Cursor *cursor);

/**
 * @brief Set cursor image
 *
 * @note This will avoid setting duplicate image. If image is NULL, hide cursor.
 */
void cursorSetImage(Cursor *cursor, const char *image);

/**
 * @brief Set cursor image surface
 *
 * @note If surface is NULL, hide cursor.
 */
void cursorSetImageSurface(Cursor *cursor, struct wlr_surface *surface, int32_t hotspotX, int32_t hotspotY);

void cursorWarp(Cursor *cursor, double lx, double ly);

/**
 * @brief Warp cursor again and refresh image
 */
void cursorRefresh(Cursor *cursor);

Output *cursorAtOutput(Cursor *cursor, struct wlr_output_layout *layout);

void xcursorReload(Cursor *cursor, const char *theme, uint32_t size);

Cursor *cursorCreate(Seat *seat, struct wl_display *display, struct wlr_output_layout *layout);

void cursorDestroy(Cursor *cursor);

#endif //SYCAMORE_CURSOR_H