#ifndef SYCAMORE_LAYER_H
#define SYCAMORE_LAYER_H

#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

class Output;

class Layer
{
public:
    struct Events
    {
        wl_signal map;
        wl_signal unmap;
    };

public:
    /**
     * @brief Create Layer
     */
    static void create(wlr_layer_surface_v1* layerSurface);

    void configure(const wlr_box& fullArea, wlr_box& usableArea);

    bool isFocusable() const;

    auto getBaseSurface() const
    {
        return m_layerSurface->surface;
    }

    Point<int32_t> getPosition() const
    {
        return {m_sceneHelper->tree->node.x, m_sceneHelper->tree->node.y};
    }

    Output* getOutput() const
    {
        return m_output;
    }

    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

public:
    Events events;

private:
    /**
     * @brief Constructor
     */
    Layer(wlr_layer_surface_v1* surface, wlr_scene_layer_surface_v1* helper);

    /**
     * @brief Destructor
     */
    ~Layer();

private:
    wlr_layer_surface_v1*       m_layerSurface;
    wlr_scene_layer_surface_v1* m_sceneHelper;
    zwlr_layer_shell_v1_layer   m_layer;
    bool                        m_lastMapState; // Update on commit

    Output*                     m_output;
    Listener                    m_outputDestroy;
    std::list<Layer*>::iterator m_iter;

private:
    Listener m_newPopup;
    Listener m_map;
    Listener m_unmap;
    Listener m_commit;
    Listener m_destroy;
};

class LayerElement final : public SceneElement
{
public:
    LayerElement(wlr_scene_node* node, Layer* layer)
        : SceneElement{SceneElement::LAYER, node}
        , m_layer{layer} {}

    ~LayerElement() override = default;

    Layer& getLayer() const
    {
        return *m_layer;
    }

private:
    Layer* m_layer;
};

}

#endif //SYCAMORE_LAYER_H