#ifndef SYCAMORE_CORE_H
#define SYCAMORE_CORE_H

#include "sycamore/defines.h"
#include "sycamore/desktop/ShellManager.h"
#include "sycamore/input/InputManager.h"
#include "sycamore/input/KeybindingManager.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputLayout.h"
#include "sycamore/scene/Scene.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>
#include <string>

NAMESPACE_SYCAMORE_BEGIN

class Core
{
public:
    struct Backend
    {
        wlr_backend* handle;
        wlr_session* session;

        Listener     newInput;
        Listener     newOutput;
        Listener     destroy;
    };

    struct Compositor
    {
        wlr_compositor* handle;

        Listener        newSurface;
        Listener        destroy;
    };

public:
    /**
     * @brief Get instance
     */
    static Core& get()
    {
        static Core instance{};
        return instance;
    }

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

    Backend*                 backend      = nullptr;
    Compositor*              compositor   = nullptr;
    OutputLayout*            outputLayout = nullptr;
    Seat*                    seat         = nullptr;

    wlr_renderer*            renderer     = nullptr;
    wlr_allocator*           allocator    = nullptr;
    wlr_presentation*        presentation = nullptr;
    wlr_linux_dmabuf_v1*     linuxDmabuf  = nullptr;
    wlr_pointer_gestures_v1* gestures     = nullptr;

    Scene::UPtr              scene;
    ShellManager::UPtr       shell;
    InputManager::UPtr       input;
    KeybindingManager::UPtr  keybinding;

    std::string              socket;

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

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_CORE_H