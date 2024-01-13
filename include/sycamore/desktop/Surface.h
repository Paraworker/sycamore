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
    static void create(wlr_surface* handle)
    {
        // Be destroyed by listener
        new Surface{handle};
    }

    Surface(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface& operator=(Surface&&) = delete;

private:
    explicit Surface(wlr_surface* handle)
    {
        m_destroy.connect(handle->events.destroy);
        m_destroy.set([this](auto)
        {
            delete this;
        });
    }

    ~Surface()
    {
        Core::instance.seat->input->rebasePointer();
    }

private:
    Listener m_destroy;
};

}

#endif //SYCAMORE_SURFACE_H