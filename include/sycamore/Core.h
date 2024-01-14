#ifndef SYCAMORE_CORE_H
#define SYCAMORE_CORE_H

#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/scene/Tree.h"
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
    void setup();

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
    wl_display*              display;
    wl_event_loop*           eventLoop;

    wlr_backend*             backend;
    wlr_session*             session;
    wlr_renderer*            renderer;
    wlr_allocator*           allocator;
    wlr_compositor*          compositor;
    wlr_linux_dmabuf_v1*     linuxDmabuf;
    wlr_pointer_gestures_v1* gestures;

    OutputLayout*            outputLayout;

    Seat*                    seat;

    scene::Tree              sceneTree;
    wlr_scene_output_layout* sceneOutputLayout;

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