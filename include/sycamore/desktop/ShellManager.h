#ifndef SYCAMORE_SHELL_MANAGER_H
#define SYCAMORE_SHELL_MANAGER_H

#include "sycamore/defines.h"
#include "sycamore/utils/List.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Layer;
class Output;
class View;

class ShellManager
{
public:
    struct FocusState
    {
        // focused view
        View* view = nullptr;

        // focused layer
        Layer* layer = nullptr;
    };

public:
    /**
     * @brief Focus a view
     */
    void setFocus(View* view);

    /**
     * @brief Focus a layer
     */
    void setFocus(Layer* layer);

    const FocusState& getFocusState() const { return m_focusState; }

    void addMappedView(View* view);

    void removeMappedView(View* view);

    const List& getMappedViewList() const { return m_mappedViewList; }

    static void maximizeRequest(View* view, bool state, Output* output);

    static void fullscreenRequest(View* view, bool state, Output* output);

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
    struct FocusUnmap
    {
        // on focused view unmap
        Listener view;

        // on focused layer unmap
        Listener layer;
    };

private:
    List       m_mappedViewList;
    FocusState m_focusState;
    FocusUnmap m_focusUnmap;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SHELL_MANAGER_H