#ifndef SYCAMORE_CORE_H
#define SYCAMORE_CORE_H

#include "sycamore/input/Cursor.h"
#include "sycamore/scene/Scene.h"
#include "sycamore/wlroots.h"

#include <memory>
#include <string>

namespace sycamore
{

class Seat;

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

    /**
     * @brief Terminate server
     */
    void terminate() const;

    /**
     * @brief Switch to virtual terminal
     */
    void switchVt(uint32_t vt) const;

public:
    wl_display*              display;
    wl_event_loop*           eventLoop;

    std::string              socket;

    wlr_backend*             backend;
    wlr_session*             session;

    wlr_renderer*            renderer;
    wlr_linux_dmabuf_v1*     linuxDmabuf;

    wlr_allocator*           allocator;

    wlr_compositor*          compositor;

    wlr_output_layout*       outputLayout;

    scene::Scene             scene;

    Cursor                   cursor;
    std::unique_ptr<Seat>    seat;
    wlr_pointer_gestures_v1* pointerGestures;

    wlr_xdg_shell*           xdgShell;
    wlr_layer_shell_v1*      layerShell;
};

inline Core core{};

}

#endif //SYCAMORE_CORE_H