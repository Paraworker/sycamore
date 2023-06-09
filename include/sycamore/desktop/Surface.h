#ifndef SYCAMORE_SURFACE_H
#define SYCAMORE_SURFACE_H

#include "sycamore/defines.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

class Surface {
public:
    static void onCreate(wlr_surface* handle) {
        new Surface{handle};
    }

public:
    Surface(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface& operator=(Surface&&) = delete;

private:
    explicit Surface(wlr_surface* handle) : m_handle(handle) {
        m_destroy.set(&handle->events.destroy, [this](void*) { delete this; });
    }

    ~Surface() { Core::instance.seat->getInput().rebasePointer(); }

private:
    wlr_surface* m_handle;

private:
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SURFACE_H