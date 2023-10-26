#ifndef SYCAMORE_LAYER_H
#define SYCAMORE_LAYER_H

#include "sycamore/defines.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

NAMESPACE_SYCAMORE_BEGIN

class Output;

class Layer {
public:
    /**
     * @brief Create Layer
     * @return nullptr on failure
     */
    static Layer* create(wlr_layer_surface_v1* layerSurface);

    void configure(const wlr_box& fullArea, wlr_box& usableArea);

    bool isFocusable() const;

    auto getBaseSurface() const { return m_layerSurface->surface; }

    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

public:
    struct {
        wl_signal map;
        wl_signal unmap;
        wl_signal destroy;
    } events{};

    wl_list link{};

private:
    Layer(wlr_layer_surface_v1* surface, wlr_scene_layer_surface_v1* helper);

    ~Layer();

private:
    wlr_layer_surface_v1*       m_layerSurface;
    wlr_scene_layer_surface_v1* m_sceneHelper;
    bool                        m_lastMapState; // Update on commit
    Output*                     m_output;
    Listener                    m_onOutputDestroy;

private:
    Listener m_newPopup;
    Listener m_map;
    Listener m_unmap;
    Listener m_commit;
    Listener m_destroy;
};

class LayerElement final : public SceneElement {
public:
    Layer* getLayer() const { return m_layer; }

private:
    LayerElement(wlr_scene_node* node, Layer* layer)
        : SceneElement(SceneElement::LAYER, node)
        , m_layer(layer) {}

    ~LayerElement() override = default;

private:
    friend class Layer;
    Layer* m_layer;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_LAYER_H