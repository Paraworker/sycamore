#ifndef SYCAMORE_CORE_H
#define SYCAMORE_CORE_H

#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/scene/Scene.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>
#include <string>

namespace sycamore
{

class Core
{
public:
    /**
     * @brief Setup server
     */
    bool setup();

    /**
     * @brief Teardown server
     */
    void teardown();

    /**
     * @brief Start backend
     */
    bool start() const;

    /**
     * @brief Run event loop
     */
    void run() const;

public:
    wl_display*              display      = nullptr;
    wl_event_loop*           eventLoop    = nullptr;

    wlr_backend*             backend      = nullptr;
    wlr_session*             session      = nullptr;
    wlr_renderer*            renderer     = nullptr;
    wlr_allocator*           allocator    = nullptr;
    wlr_compositor*          compositor   = nullptr;
    wlr_presentation*        presentation = nullptr;
    wlr_linux_dmabuf_v1*     linuxDmabuf  = nullptr;
    wlr_pointer_gestures_v1* gestures     = nullptr;

    OutputLayout*            outputLayout = nullptr;
    Seat*                    seat         = nullptr;

    Scene::UPtr              scene;

    std::string              socket;

public:
    static Core instance;

private:
    /**
     * @brief Constructor
     */
    Core() = default;

    /**
     * @brief Destructor
     */
    ~Core() = default;
};

}

#endif //SYCAMORE_CORE_H