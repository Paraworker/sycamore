#ifndef SYCAMORE_SURFACE_H
#define SYCAMORE_SURFACE_H

#include "sycamore/input/InputManager.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"
#include "sycamore/Core.h"

namespace sycamore
{

class Surface
{
public:
    /**
     * @brief Create Surface
     */
    static Surface* create(wlr_surface* handle)
    {
        return new Surface{handle};
    }

    Surface(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface& operator=(Surface&&) = delete;

private:
    explicit Surface(wlr_surface* handle) : m_handle{handle}
    {
        m_destroy
        .connect(handle->events.destroy)
        .set([this](void*)
        {
            delete this;
        });
    }

    ~Surface()
    {
        Core::instance.seat->getInput().rebasePointer();
    }

private:
    wlr_surface* m_handle;

private:
    Listener m_destroy;
};

}

#endif //SYCAMORE_SURFACE_H