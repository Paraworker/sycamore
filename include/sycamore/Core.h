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
    * @brief Constructor
    */
    Core();

    /**
     * @brief Destructor
     */
    ~Core();

    /**
     * @brief Start backend
     */
    void start();

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

    OutputLayout*            outputLayout;
    Seat*                    seat;

    scene::Tree              sceneTree;
    wlr_scene_output_layout* sceneOutputLayout;

    wlr_linux_dmabuf_v1*     linuxDmabuf;
    wlr_pointer_gestures_v1* gestures;

    std::string              socket;
};

inline Core core{};

}

#endif //SYCAMORE_CORE_H