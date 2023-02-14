#ifndef SYCAMORE_VIEW_H
#define SYCAMORE_VIEW_H

#include <stdbool.h>
#include <wlr/util/box.h>
#include "sycamore/output/scene.h"

typedef struct View          View;
typedef struct ViewInterface ViewInterface;
typedef struct ViewPtr       ViewPtr;
typedef enum ViewType        ViewType;
typedef struct Output        Output;

enum ViewType {
    VIEW_TYPE_UNKNOWN,
    VIEW_TYPE_XDG_SHELL,
    VIEW_TYPE_XWAYLAND,
};

struct ViewPtr {
    View *view;
    struct wl_list link;    // view::ptrs
};

struct ViewInterface {
    void (*destroy)(View *view);
    void (*map)(View *view);
    void (*unmap)(View *view);
    void (*setActivated)(View *view, bool activated);
    void (*setSize)(View *view, uint32_t width, uint32_t height);
    void (*setFullscreen)(View *view, bool fullscreen);
    void (*setMaximized)(View *view, bool maximized);
    void (*setResizing)(View *view, bool resizing);
    void (*getGeometry)(View *view, struct wlr_box *box);
    void (*close)(View *view);
};

// base view
struct View {
    SceneDescriptorType sceneDesc;    // must be first
    const ViewInterface *interface;
    struct wlr_surface  *wlrSurface;
    ViewType            type;
    int x, y;

    struct wlr_scene_tree *sceneTree;

    struct wl_list link;
    struct wl_list ptrs;

    bool mapped;
    bool isMaximized;
    bool isFullscreen;

    struct {
        struct wlr_box maximize;
        struct wlr_box fullscreen;
    } restore;
};

void viewInit(View *view, ViewType type, struct wlr_surface *surface, const ViewInterface *interface);

void viewMap(View *view, struct wlr_output *fullscreenOutput, bool maximized, bool fullscreen);

void viewUnmap(View *view);

void viewDestroy(View *view);

void viewMoveTo(View *view, int x, int y);

Output *viewGetMainOutput(View *view);

void viewSetFullscreen(View *view, Output *output, bool fullscreen);

void viewSetMaximized(View *view, Output *output, bool maximized);

void viewSetFocus(View *view);

void viewSetToOutputCenter(View *view, Output *output);

void viewPtrConnect(ViewPtr *ptr, View *view);

void viewPtrDisconnect(ViewPtr *ptr);

#endif //SYCAMORE_VIEW_H