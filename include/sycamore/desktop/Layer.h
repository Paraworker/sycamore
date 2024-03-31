#ifndef SYCAMORE_LAYER_H
#define SYCAMORE_LAYER_H

#include "sycamore/scene/Element.h"
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
     * @brief Constructor
     */
    explicit Layer(wlr_layer_surface_v1* handle);

    /**
     * @brief Destructor
     */
    ~Layer();

    void configure(const wlr_box& fullArea, wlr_box& usableArea);

    bool isFocusable() const;

    auto baseSurface() const
    {
        return m_handle->surface;
    }

    Point<int32_t> position() const
    {
        return {m_sceneHelper->tree->node.x, m_sceneHelper->tree->node.y};
    }

    Output* output() const
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
    wlr_layer_surface_v1*       m_handle;
    wlr_scene_layer_surface_v1* m_sceneHelper;
    zwlr_layer_shell_v1_layer   m_layer;
    bool                        m_lastMapState; // Update on commit

    Output*                     m_output;
    Listener                    m_outputDestroy;

    std::list<Layer*>::iterator m_iter;

    Listener                    m_newPopup;
    Listener                    m_map;
    Listener                    m_unmap;
    Listener                    m_commit;
    Listener                    m_destroy;
};

struct LayerElement final : scene::Element
{
    Layer&   layer;
    Listener destroy;

    LayerElement(wlr_scene_node& node, Layer& layer)
        : Element{LAYER}, layer{layer}
    {
        // attach node
        node.data = this;

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(node.events.destroy);
    }
};

}

#endif //SYCAMORE_LAYER_H