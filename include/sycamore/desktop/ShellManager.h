#ifndef SYCAMORE_SHELL_MANAGER_H
#define SYCAMORE_SHELL_MANAGER_H

#include "sycamore/utils/Listener.h"

#include <list>

namespace sycamore
{

class Layer;
class Output;
class Toplevel;

class ShellManager
{
public:
    struct FocusState
    {
        Toplevel* toplevel; // focused Toplevel
        Layer*    layer;    // focused Layer
    };

public:
    /**
     * @brief Focus a toplevel
     */
    void setFocus(Toplevel& toplevel);

    /**
     * @brief Focus a layer
     */
    void setFocus(Layer& layer);

    const FocusState& getFocusState() const
    {
        return m_focusState;
    }

    void onToplevelMap(Toplevel& toplevel);

    void onToplevelUnmap(Toplevel& toplevel);

    void onLayerMap(Layer& layer);

    void onLayerUnmap(Layer& layer);

    void cycleToplevel();

    void fullscreenRequest(Toplevel& toplevel, bool state, Output* output);

    static void maximizeRequest(Toplevel& toplevel, bool state, Output* output);

public:
    static ShellManager instance;

private:
    /**
     * @brief Constructor
     */
    ShellManager();

    /**
     * @brief Destructor
     */
    ~ShellManager();

private:
    std::list<Toplevel*> m_mappedToplevels;
    FocusState           m_focusState;
    size_t               m_fullscreenCount;
};

}

#endif //SYCAMORE_SHELL_MANAGER_H