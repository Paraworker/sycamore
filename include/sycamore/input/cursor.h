#ifndef SYCAMORE_CURSOR_H
#define SYCAMORE_CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>

typedef struct Cursor Cursor;
typedef struct Output Output;
typedef struct Seat   Seat;

struct Cursor {
    struct wlr_cursor              *wlrCursor;
    struct wlr_xcursor_manager     *xcursorManager;
    struct wlr_pointer_gestures_v1 *gestures;

    bool       enabled;
    const char *image;

    size_t pressedButtonCount;

    struct wl_listener frame;

    struct wl_listener motion;
    struct wl_listener motionAbsolute;
    struct wl_listener button;
    struct wl_listener axis;

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

void cursorDisable(Cursor *cursor);

void cursorEnable(Cursor *cursor);

/**
 * @brief Set cursor image
 *
 * @note Pass image NULL will hide cursor
 */
void cursorSetImage(Cursor *cursor, const char *image);

/**
 * @brief Set cursor image surface
 *
 */
void cursorSetSurface(Cursor *cursor, struct wlr_surface *surface, int32_t hotspotX, int32_t hotspotY);

void cursorRefresh(Cursor *cursor);

void cursorRebase(Cursor *cursor);

void cursorWarp(Cursor *cursor, double lx, double ly);

Output *cursorAtOutput(Cursor *cursor);

void xcursorReload(Cursor *cursor, const char *theme, uint32_t size);

Cursor *cursorCreate(Seat *seat, struct wl_display *display, struct wlr_output_layout *layout);

void cursorDestroy(Cursor *cursor);

#endif //SYCAMORE_CURSOR_H