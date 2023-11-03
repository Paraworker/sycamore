#ifndef SYCAMORE_CORE_H
#define SYCAMORE_CORE_H

#include "sycamore/defines.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/scene/Scene.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>
#include <string>

NAMESPACE_SYCAMORE_BEGIN

class Core {
public:
    /**
     * @brief Initialize
     */
    bool init();

    /**
     * @brief Uninitialize
     */
    void uninit();

    /**
     * @brief Start backend
     */
    bool start() const;

    /**
     * @brief Run event loop
     */
    void run() const;

public: // libwayland
    wl_display*    display   = nullptr;
    wl_event_loop* eventLoop = nullptr;

public: // wlr
    wlr_backend*             backend      = nullptr;
    wlr_allocator*           allocator    = nullptr;
    wlr_compositor*          compositor   = nullptr;
    wlr_renderer*            renderer     = nullptr;
    wlr_session*             session      = nullptr;
    wlr_presentation*        presentation = nullptr;
    wlr_linux_dmabuf_v1*     dmabuf       = nullptr;
    wlr_pointer_gestures_v1* gestures     = nullptr;

public:
    OutputLayout::UPtr outputLayout;
    Scene::UPtr        scene;
    Seat::UPtr         seat;

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

private:
    std::string m_socket;

private:
    Listener m_newInput;
    Listener m_newOutput;
    Listener m_newSurface;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_CORE_H