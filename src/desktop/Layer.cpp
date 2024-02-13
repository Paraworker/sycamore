#include "sycamore/desktop/Layer.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

struct LayerPopup : Popup::Handler
{
    Layer& layer;

    explicit LayerPopup(Layer& layer)
        : layer{layer}
    {}

    ~LayerPopup() override = default;

    void unconstrain(Popup& popup) override
    {
        auto pos = layer.position();
        auto box = layer.output()->relativeGeometry();

        box.x = -pos.x;
        box.y = -pos.y;

        popup.unconstrainFromBox(box);
    }
};

Layer::Layer(wlr_layer_surface_v1* handle)
    : m_handle{handle}
    , m_sceneHelper{wlr_scene_layer_surface_v1_create(core.scene.treeForLayer(handle->pending.layer), handle)}
    , m_layer{handle->pending.layer}
    , m_lastMapState{false}
    , m_output{static_cast<Output*>(handle->output->data)}
 {
    new LayerElement{m_sceneHelper->tree->node, *this};

    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);

    // Link to output
    auto& layerList = m_output->layerList[m_layer];
    m_iter = layerList.emplace(layerList.end(), this);

    m_outputDestroy = [this](auto)
    {
        m_handle->output = nullptr;

        // Unlink output
        m_outputDestroy.disconnect();
        m_output->layerList[m_layer].erase(m_iter);
        m_output = nullptr;
    };
    m_outputDestroy.connect(m_output->events.destroy);

    m_newPopup = [this](void* data)
    {
        new Popup{static_cast<wlr_xdg_popup*>(data), m_sceneHelper->tree, std::make_shared<LayerPopup>(*this)};
    };
    m_newPopup.connect(handle->events.new_popup);

    m_map = [this](auto)
    {
        windowManager.mapLayer(*this);
    };
    m_map.connect(handle->surface->events.map);

    m_unmap = [this](auto)
    {
        windowManager.unmapLayer(*this);
    };
    m_unmap.connect(handle->surface->events.unmap);

    m_commit = [this](auto)
    {
        uint32_t committed   = m_handle->current.committed;
        bool     mapped      = m_handle->surface->mapped;
        bool     needArrange = false;
        bool     needRebase  = false;

        if (committed)
        {
            // Layer type changed
            if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER)
            {
                auto& oldList = m_output->layerList[m_layer];
                m_layer = m_handle->current.layer;
                auto& newList = m_output->layerList[m_layer];

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
    };
    m_commit.connect(handle->surface->events.commit);

    m_destroy = [this](auto)
    {
        delete this;
    };
    m_destroy.connect(handle->events.destroy);
}

Layer::~Layer()
{
    if (m_output)
    {
        // Unlink output
        m_output->layerList[m_layer].erase(m_iter);
        m_output->arrangeLayers();
    }
}

void Layer::configure(const wlr_box& fullArea, wlr_box& usableArea)
{
    wlr_scene_layer_surface_v1_configure(m_sceneHelper, &fullArea, &usableArea);
}

bool Layer::isFocusable() const
{
    return (m_handle->current.keyboard_interactive) &&
           (m_handle->current.layer > ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
}

}