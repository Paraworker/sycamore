#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <list>

#define LAYER_COUNT 4

namespace sycamore
{

class Layer;

class Output
{
public:
    struct Events
    {
        wl_signal destroy;
    };

public:
    /**
     * @brief Create Output
     * @return nullptr on failure
     */
    static Output* create(wlr_output* handle);

    /**
     * @brief Return the output name
     */
    auto name() noexcept
    {
        return m_handle->name;
    }

    bool apply();

    /**
     * @brief Get geometry in local layout
     * @return {0, 0, width, height}
     */
    wlr_box getRelativeGeometry() const;

    /**
     * @brief Get geometry in output layout
     * @return {x, y, width, height}
     */
    wlr_box getLayoutGeometry() const;

    /**
     * @brief Center cursor on this output
     */
    void ensureCursor() const;

    void arrangeLayers();

    auto getHandle() noexcept
    {
        return m_handle;
    }

    auto getSceneOutput() noexcept
    {
        return m_sceneOutput;
    }

    const wlr_box& getUsableArea() const noexcept
    {
        return m_usableArea;
    }

    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;

private:
    /**
     * @brief Constructor
     */
    Output(wlr_output* handle, wlr_scene_output* sceneOutput);

    /**
     * @brief Destructor
     */
    ~Output();

public:
    Events            events;
    std::list<Layer*> layers[LAYER_COUNT];

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