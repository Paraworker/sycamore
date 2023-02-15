#ifndef SYCAMORE_SEAT_H
#define SYCAMORE_SEAT_H

#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include "sycamore/desktop/view.h"

typedef struct Cursor     Cursor;
typedef struct Drag       Drag;
typedef struct DragIcon   DragIcon;
typedef struct Layer      Layer;
typedef struct Seat       Seat;

typedef struct SeatopImpl SeatopImpl;
typedef enum SeatopMode   SeatopMode;

typedef union SeatopData               SeatopData;
typedef struct SeatopPointerDownData   SeatopPointerDownData;
typedef struct SeatopPointerMoveData   SeatopPointerMoveData;
typedef struct SeatopPointerResizeData SeatopPointerResizeData;

enum SeatopMode {
    BLANK,
    BASIC_FULL,
    BASIC_POINTER_NO,
    POINTER_DOWN,
    POINTER_MOVE,
    POINTER_RESIZE,
};

struct SeatopPointerDownData {
    struct wlr_surface *surface;
    struct wl_listener surfaceDestroy;
    double dx, dy;
    Seat *seat;
};

struct SeatopPointerMoveData {
    ViewPtr viewPtr;
    double dx, dy;
};

struct SeatopPointerResizeData {
    ViewPtr viewPtr;
    double dx, dy;
    struct wlr_box grabGeobox;
    uint32_t edges;
};

union SeatopData {
    SeatopPointerDownData   pointerDown;
    SeatopPointerMoveData   pointerMove;
    SeatopPointerResizeData pointerResize;
};

struct SeatopImpl {
    void (*pointerButton)(Seat *seat, struct wlr_pointer_button_event *event);
    void (*pointerMotion)(Seat *seat, uint32_t timeMsec);
    void (*pointerRebase)(Seat *seat);
    void (*end)(Seat *seat);
    enum SeatopMode mode;
};

struct DragIcon {
    SceneDescriptorType   sceneDesc; // must be first
    struct wlr_drag_icon  *wlrDragIcon;
    struct wlr_scene_tree *tree;
    struct wl_listener    destroy;
};

struct Drag {
    struct wlr_drag    *wlrDrag;
    struct wl_listener destroy;
    Seat               *seat;
};

struct Seat {
    struct wlr_seat *wlrSeat;
    Cursor          *cursor;
    struct wl_list  devices;

    const SeatopImpl *seatopImpl;
    SeatopData       seatopData;

    Layer            *focusedLayer;

    struct wl_listener requestSetCursor;
    struct wl_listener requestSetSelection;
    struct wl_listener requestSetPrimarySelection;
    struct wl_listener requestStartDrag;
    struct wl_listener startDrag;
    struct wl_listener destroy;
};

void onBackendNewInput(struct wl_listener *listener, void *data);

Seat *seatCreate(struct wl_display *display, struct wlr_output_layout *layout);

void seatDestroy(Seat *seat);

void seatUpdateCapabilities(Seat *seat);

void seatPointerUpdateFocus(Seat *seat, uint32_t timeMsec);

void seatSetKeyboardFocus(Seat *seat, struct wlr_surface *surface);

void seatDragIconUpdatePosition(Seat *seat, DragIcon *icon);

static inline void seatopPointerButton(Seat *seat, struct wlr_pointer_button_event *event) {
    if (seat->seatopImpl->pointerButton) {
        seat->seatopImpl->pointerButton(seat, event);
    }
}

static inline void seatopPointerMotion(Seat *seat, uint32_t time_msec) {
    if (seat->seatopImpl->pointerMotion) {
        seat->seatopImpl->pointerMotion(seat, time_msec);
    }
}

static inline void seatopPointerRebase(Seat *seat) {
    if (seat->seatopImpl->pointerRebase) {
        seat->seatopImpl->pointerRebase(seat);
    }
}

static inline void seatopEnd(Seat *seat) {
    if (seat->seatopImpl && seat->seatopImpl->end) {
        seat->seatopImpl->end(seat);
    }

    seat->seatopImpl = NULL;
}

void seatopSetBlank(Seat *seat);

void seatopSetBasicFull(Seat *seat);

void seatopSetBasicPointerNo(Seat *seat);

void seatopSetPointerDown(Seat *seat, struct wlr_surface *surface, double sx, double sy);

void seatopSetPointerMove(Seat *seat, View *view);

void seatopSetPointerResize(Seat *seat, View *view, uint32_t edges);

bool seatopPointerInteractiveModeCheck(Seat *seat, View *view, SeatopMode mode);

#endif //SYCAMORE_SEAT_H