#ifndef SYCAMORE_LAYER_SHELL_H
#define SYCAMORE_LAYER_SHELL_H

#include <wlr/types/wlr_layer_shell_v1.h>
#include "sycamore/server.h"

struct sycamore_layer_shell {
    struct wlr_layer_shell_v1* wlr_layer_shell;
    struct wl_listener new_layer_shell_surface;

    struct sycamore_server* server;
};

struct sycamore_layer_shell* sycamore_layer_shell_create(
        struct sycamore_server* server, struct wl_display* display);
void sycamore_layer_shell_destroy(struct sycamore_layer_shell* layer_shell);
void handle_sycamore_layer_map(struct wl_listener *listener, void *data);
void handle_sycamore_layer_unmap(struct wl_listener *listener, void *data);
void handle_sycamore_layer_destroy(struct wl_listener *listener, void *data);


#endif //SYCAMORE_LAYER_SHELL_H
