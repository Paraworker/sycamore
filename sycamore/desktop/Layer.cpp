#include "sycamore/desktop/WindowManager.h"
#include "sycamore/desktop/Layer.h"
#include "sycamore/desktop/Popup.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Layer* Layer::create(wlr_layer_surface_v1* layerSurface) {
    // Confirm output
    if (!layerSurface->output) {
        auto output = Core::instance.seat->getCursor().atOutput();
        if (!output) {
            spdlog::error("No output under cursor for layerSurface");
            wlr_layer_surface_v1_destroy(layerSurface);
            return nullptr;
        }

        layerSurface->output = output->getHandle();
    }

    // Create scene helper
    auto helper = wlr_scene_layer_surface_v1_create(Core::instance.scene->getLayerTree(layerSurface->pending.layer), layerSurface);
    if (!helper) {
        spdlog::error("Create wlr_scene_layer_surface_v1 failed!");
        wlr_layer_surface_v1_destroy(layerSurface);
        return nullptr;
    }


    // Create Layer
    return new Layer{layerSurface, helper};
}

Layer::Layer(wlr_layer_surface_v1* layerSurface, wlr_scene_layer_surface_v1* helper)
    : m_layerSurface(layerSurface)
    , m_sceneHelper(helper)
    , m_output(static_cast<Output*>(layerSurface->output->data))
    , m_lastMapState(false) {
    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);
    wl_signal_init(&events.destroy);

    // Link to output
    wl_list_insert(&m_output->layers[m_layerSurface->pending.layer], &link);
    m_onOutputDestroy.set(&m_output->events.destroy, [this](void*) {
        m_layerSurface->output = nullptr;

        // Unlink output
        wl_list_remove(&link);

        m_output = nullptr;
        m_onOutputDestroy.disconnect();
    });

    m_newPopup.set(&layerSurface->events.new_popup, [this](void* data) {
        Popup::create(static_cast<wlr_xdg_popup*>(data), m_sceneHelper->tree);
    });

    m_map.set(&layerSurface->surface->events.map, [this](void*) {
        if (isFocusable()) {
            WindowManager::instance.setFocus(this);
        }

        // emit signal
        wl_signal_emit_mutable(&events.map, nullptr);
    });

    m_unmap.set(&layerSurface->surface->events.unmap, [this](void*) {
        // emit signal
        wl_signal_emit_mutable(&events.unmap, nullptr);
    });

    m_commit.set(&layerSurface->surface->events.commit, [this](void*) {
        uint32_t committed   = m_layerSurface->current.committed;
        bool     mapped      = m_layerSurface->surface->mapped;
        bool     needArrange = false;
        bool     needRebase  = false;

        if (committed) {
            // layer type changed
            if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
                auto newLayer = m_layerSurface->current.layer;
                wlr_scene_node_reparent(&m_sceneHelper->tree->node, Core::instance.scene->getLayerTree(newLayer));

                wl_list_remove(&link);
                wl_list_insert(&m_output->layers[newLayer], &link);
            }

            // only need rebase after mapped
            if (mapped) {
                needRebase = true;
            }

            needArrange = true;
        }

        if (mapped != m_lastMapState) {
            m_lastMapState = mapped;

            // need rebase if map state changed
            needRebase = true;
        }

        if (needArrange) {
            m_output->arrangeLayers();
        }

        if (needRebase) {
            Core::instance.seat->getInput().rebasePointer();
        }
    });

    m_destroy.set(&layerSurface->surface->events.destroy, [this](void*) {
        // emit signal first
        wl_signal_emit_mutable(&events.destroy, nullptr);

        if (m_output) {
            // Unlink output
            wl_list_remove(&link);
            m_output->arrangeLayers();
        }

        delete this;
    });

    // Create LayerElement
    new LayerElement{&helper->tree->node, this};
}

Layer::~Layer() = default;

void Layer::configure(const wlr_box& fullArea, wlr_box& usableArea) {
    wlr_scene_layer_surface_v1_configure(m_sceneHelper, &fullArea, &usableArea);
}

bool Layer::isFocusable() const {
    return (m_layerSurface->current.keyboard_interactive) &&
           (m_layerSurface->current.layer > ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
}

NAMESPACE_SYCAMORE_END