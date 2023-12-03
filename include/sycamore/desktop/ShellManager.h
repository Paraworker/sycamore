#ifndef SYCAMORE_SHELL_MANAGER_H
#define SYCAMORE_SHELL_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/utils/Listener.h"

#include <list>
#include <memory>

NAMESPACE_SYCAMORE_BEGIN

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

    using UPtr = std::unique_ptr<ShellManager>;

public:
    /**
     * @brief Constructor
     */
    ShellManager();

    /**
     * @brief Destructor
     */
    ~ShellManager();

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

    ShellManager(const ShellManager&) = delete;
    ShellManager(ShellManager&&) = delete;
    ShellManager& operator=(const ShellManager&) = delete;
    ShellManager& operator=(ShellManager&&) = delete;

private:
    std::list<Toplevel*> m_mappedToplevels;
    FocusState           m_focusState;
    size_t               m_fullscreenCount;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SHELL_MANAGER_H