#ifndef SYCAMORE_SERVER_H
#define SYCAMORE_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include "sycamore/desktop/view.h"

typedef struct Scene             Scene;
typedef struct Seat              Seat;
typedef struct XdgShell          XdgShell;
typedef struct LayerShell        LayerShell;
typedef struct KeybindingManager KeybindingManager;

typedef struct Server            Server;

// The global server instance.
extern Server server;

struct Server {
    struct wl_display    *wlDisplay;

    struct wlr_backend   *backend;
    struct wlr_session   *session;
    struct wlr_renderer  *renderer;
    struct wlr_allocator *allocator;

    struct wlr_compositor    *compositor;
    struct wlr_output_layout *outputLayout;
    struct wlr_presentation  *presentation;

    Scene             *scene;
    Seat              *seat;
    XdgShell          *xdgShell;
    LayerShell        *layerShell;
    KeybindingManager *keybindingManager;

    struct wl_listener backendNewInput;
    struct wl_listener backendNewOutput;
    struct wl_listener outputLayoutChange;

    struct wl_list allOutputs;
    struct wl_list mappedViews;
    struct ViewPtr focusedView;

    const char *socket;
};

bool serverInit();

/**
 * @brief Start the backend
 *
 * @return bool
 */
bool serverStart();

/**
 * @brief Run the wayland event loop
 */
void serverRun();

void serverUninit();

#endif //SYCAMORE_SERVER_H