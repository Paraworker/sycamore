#include "sycamore/desktop/Layer.h"

#include "sycamore/desktop/ShellManager.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

void Layer::create(wlr_layer_surface_v1* layerSurface)
{
    // Confirm output
    if (!layerSurface->output)
    {
        auto output = core.seat->cursor.atOutput();
        if (!output)
        {
            spdlog::error("No output under cursor for layerSurface");
            wlr_layer_surface_v1_destroy(layerSurface);
            return;
        }

        layerSurface->output = output->getHandle();
    }

    // Create scene helper
    auto helper = wlr_scene_layer_surface_v1_create(core.scene.treeForLayer(layerSurface->pending.layer), layerSurface);
    if (!helper)
    {
        spdlog::error("Create wlr_scene_layer_surface_v1 failed!");
        wlr_layer_surface_v1_destroy(layerSurface);
        return;
    }

    // Be destroyed by listener
    new Layer{layerSurface, helper};
}

Layer::Layer(wlr_layer_surface_v1* layerSurface, wlr_scene_layer_surface_v1* helper)
    : m_layerSurface{layerSurface}
    , m_sceneHelper{helper}
    , m_layer{m_layerSurface->pending.layer}
    , m_lastMapState{false}
    , m_output{static_cast<Output*>(layerSurface->output->data)}
 {
    // Create LayerElement
    new LayerElement{&helper->tree->node, *this};

    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);

    // Link to output
    auto& layerList = m_output->layers[m_layer];
    m_iter = layerList.emplace(layerList.end(), this);

    m_outputDestroy.notify([this](auto)
    {
        m_layerSurface->output = nullptr;

        // Unlink output
        m_outputDestroy.disconnect();
        m_output->layers[m_layer].erase(m_iter);
        m_output = nullptr;
    });
    m_outputDestroy.connect(m_output->events.destroy);

    m_newPopup.notify([this](void* data)
    {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_sceneHelper->tree, std::make_shared<Popup::LayerHandler>(*this));
    });
    m_newPopup.connect(layerSurface->events.new_popup);

    m_map.notify([this](auto)
    {
        shellManager.onLayerMap(*this);
        wl_signal_emit_mutable(&events.map, nullptr);
    });
    m_map.connect(layerSurface->surface->events.map);

    m_unmap.notify([this](auto)
    {
        shellManager.onLayerUnmap(*this);
        wl_signal_emit_mutable(&events.unmap, nullptr);
    });
    m_unmap.connect(layerSurface->surface->events.unmap);

    m_commit.notify([this](auto)
    {
        uint32_t committed   = m_layerSurface->current.committed;
        bool     mapped      = m_layerSurface->surface->mapped;
        bool     needArrange = false;
        bool     needRebase  = false;

        if (committed)
        {
            // Layer type changed
            if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER)
            {
                auto& oldList = m_output->layers[m_layer];
                m_layer = m_layerSurface->current.layer;
                auto& newList = m_output->layers[m_layer];

                wlr_scene_node_reparent(&m_sceneHelper->tree->node, core.scene.treeForLayer(m_layer));
                newList.splice(newList.end(), oldList, m_iter);
            }

            // Only need rebase after mapping
            if (mapped)
            {
                needRebase = true;
            }

            needArrange = true;
        }

        if (mapped != m_lastMapState)
        {
            m_lastMapState = mapped;

            // need rebase if map state changed
            needRebase = true;
        }

        if (needArrange)
        {
            m_output->arrangeLayers();
        }

        if (needRebase)
        {
            core.seat->input->rebasePointer();
        }
    });
    m_commit.connect(layerSurface->surface->events.commit);

    m_destroy.notify([this](auto)
    {
        delete this;
    });
    m_destroy.connect(layerSurface->events.destroy);
}

Layer::~Layer()
{
    if (m_output)
    {
        // Unlink output
        m_output->layers[m_layer].erase(m_iter);
        m_output->arrangeLayers();
    }
}

void Layer::configure(const wlr_box& fullArea, wlr_box& usableArea)
{
    wlr_scene_layer_surface_v1_configure(m_sceneHelper, &fullArea, &usableArea);
}

bool Layer::isFocusable() const
{
    return (m_layerSurface->current.keyboard_interactive) &&
           (m_layerSurface->current.layer > ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
}

}