#ifndef SYCAMORE_WINDOW_MANAGER_H
#define SYCAMORE_WINDOW_MANAGER_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/desktop/XdgToplevel.h"
#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

class Layer;
class Output;
class Toplevel;

class WindowManager
{
public:
    struct FocusState
    {
        Toplevel* toplevel; // focused Toplevel
        Layer*    layer;    // focused Layer
    };

public:
    /**
     * @brief Constructor
     */
    WindowManager();

    /**
     * @brief Destructor
     */
    ~WindowManager();

    const auto& focusState() const
    {
        return m_focusState;
    }

    void focusToplevel(Toplevel& toplevel);
    void focusLayer(Layer& layer);

    void mapToplevel(Toplevel& toplevel, bool maximized, bool fullscreen);
    void mapLayer(Layer& layer);

    void unmapToplevel(Toplevel& toplevel);
    void unmapLayer(Layer& layer);

    void fullscreenRequest(Toplevel& toplevel, const Output& output);
    void unfullscreenRequest(Toplevel& toplevel);

    static void maximizeRequest(Toplevel& toplevel, const Output& output);
    static void unmaximizeRequest(Toplevel& toplevel);

    void cycleToplevel();
    void closeFocusedToplevel() const;

private:
    std::list<Toplevel*> m_mappedToplevels;
    FocusState           m_focusState;
    size_t               m_fullscreenCount;
};

inline WindowManager windowManager{};

}

#endif //SYCAMORE_WINDOW_MANAGER_H