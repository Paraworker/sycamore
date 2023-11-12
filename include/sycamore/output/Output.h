#ifndef SYCAMORE_OUTPUT_H
#define SYCAMORE_OUTPUT_H

#include "sycamore/defines.h"
#include "sycamore/wlroots.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class Layer;

class Output
{
public:
    /**
     * @brief Create Output
     * @return nullptr on failure
     */
    static Output* create(wlr_output* handle);

    const char* name() const { return m_handle->name; }

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

    auto* getHandle() const { return m_handle; }

    auto* getSceneOutput() const { return m_sceneOutput; }

    const wlr_box& getUsableArea() const { return m_usableArea; }

    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;

private:
    Output(wlr_output* handle, wlr_scene_output* sceneOutput);

    ~Output();

public:
    struct
    {
        wl_signal destroy;
    } events{};

public:
    wl_list layers[LAYER_SHELL_LAYER_NUM]{}; // Layer::link
    wl_list link{};

private:
    wlr_output*       m_handle;
    wlr_scene_output* m_sceneOutput;
    wlr_box           m_usableArea;

private:
    Listener m_frame;
    Listener m_requestState;
    Listener m_destroy;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_OUTPUT_H