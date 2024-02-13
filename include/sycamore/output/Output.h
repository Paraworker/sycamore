#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

inline constexpr auto LAYER_COUNT = 4;

class Layer;

class Output
{
public:
    struct Events
    {
        wl_signal destroy;
    };

    using Iter = std::list<Output>::iterator;

public:
    /**
     * @brief Constructor
     */
    Output(wlr_output* handle, wlr_scene_output* sceneOutput);

    /**
     * @brief Destructor
     */
    ~Output();

    /**
     * @brief Get the output name
     */
    auto name() const
    {
        return m_handle->name;
    }

    bool apply();

    /**
     * @brief  Get geometry in local layout
     * @return {0, 0, width, height}
     */
    wlr_box relativeGeometry() const;

    /**
     * @brief  Get geometry in output layout
     * @return {x, y, width, height}
     */
    wlr_box layoutGeometry() const;

    /**
     * @brief Center cursor on this output
     */
    void ensureCursor() const;

    void arrangeLayers();

    auto getHandle()
    {
        return m_handle;
    }

    const auto& usableArea() const
    {
        return m_usableArea;
    }

    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;

public:
    Events            events;
    std::list<Layer*> layerList[LAYER_COUNT];
    Iter              iter;

private:
    wlr_output*       m_handle;
    wlr_scene_output* m_sceneOutput;
    wlr_box           m_usableArea;

    Listener          m_frame;
    Listener          m_requestState;
    Listener          m_destroy;
};

}

#endif //SYCAMORE_OUTPUT_H