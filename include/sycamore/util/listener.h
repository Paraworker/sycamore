#ifndef SYCAMORE_LISTENER_H
#define SYCAMORE_LISTENER_H

#include <wayland-server-core.h>

static inline void listener_connect(struct wl_listener *listener,
        struct wl_signal *signal, wl_notify_func_t callback) {
    listener->notify = callback;
    wl_signal_add(signal, listener);
}

static inline void listener_disconnect(struct wl_listener *listener) {
    wl_list_remove(&listener->link);
}

#endif //SYCAMORE_LISTENER_H
