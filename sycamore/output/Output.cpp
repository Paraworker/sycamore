#include "sycamore/desktop/Layer.h"
#include "sycamore/output/Output.h"
#include "sycamore/utils/box.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

Output* Output::create(wlr_output* handle) {
    spdlog::info("New Output: {}", handle->name);

    if (!wlr_output_init_render(handle, Core::instance.allocator, Core::instance.renderer)) {
        spdlog::error("Output: {} init render failed", handle->name);
        wlr_output_destroy(handle);
        return nullptr;
    }

    auto sceneOutput = wlr_scene_output_create(Core::instance.scene->getHandle(), handle);
    if (!sceneOutput) {
        spdlog::error("Output: {} create wlr_scene_output failed", handle->name);
        wlr_output_destroy(handle);
        return nullptr;
    }

    auto output = new Output{handle, sceneOutput};

    Core::instance.outputLayout->add(output);

    // TODO: configurable
    output->apply();

    return output;
}

Output::Output(wlr_output* handle, wlr_scene_output* sceneOutput)
    : m_handle(handle)
    , m_sceneOutput(sceneOutput)
    , m_usableArea(getLayoutGeometry()) {
    wl_signal_init(&events.destroy);

    for (auto& layer : layers) {
        wl_list_init(&layer);
    }

    handle->data = this;

    m_frame.set(&handle->events.frame, [this](void*) {
        // Render the scene if needed and commit the output
        wlr_scene_output_commit(m_sceneOutput, nullptr);

        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        wlr_scene_output_send_frame_done(m_sceneOutput, &now);
    });

    m_requestState.set(&handle->events.request_state, [this](void* data) {
        wlr_output_commit_state(m_handle, static_cast<wlr_output_event_request_state*>(data)->state);
    });

    m_destroy.set(&handle->events.destroy, [this](void*) {
        wl_signal_emit_mutable(&events.destroy, nullptr);
        Core::instance.outputLayout->remove(this);
        delete this;
    });
}

Output::~Output() {
    m_handle->data = nullptr;
}

bool Output::apply() {
    wlr_output_state state{};
    wlr_output_state_init(&state);

    wlr_output_state_set_enabled(&state, true);

    auto mode = wlr_output_preferred_mode(m_handle);
    if (mode) {
        wlr_output_state_set_mode(&state, mode);
    }

    if (!wlr_output_commit_state(m_handle, &state)) {
        spdlog::error("Output: {} commit state failed", m_handle->name);
        wlr_output_state_finish(&state);
        return false;
    }

    wlr_output_state_finish(&state);

    // If this is the first output, center cursor.
    if (Core::instance.outputLayout->getOutputNum() == 1) {
        ensureCursor();
    }

    arrangeLayers();
    return true;
}

wlr_box Output::getRelativeGeometry() const {
    wlr_box box{};
    wlr_output_effective_resolution(m_handle, &box.width, &box.height);
    return box;
}

wlr_box Output::getLayoutGeometry() const {
    wlr_box box{};
    wlr_output_effective_resolution(m_handle, &box.width, &box.height);

    auto layoutOutput = wlr_output_layout_get(Core::instance.outputLayout->getHandle(), m_handle);
    if (!layoutOutput) {
        return box;
    }

    box.x = layoutOutput->x;
    box.y = layoutOutput->y;

    return box;
}

void Output::ensureCursor() const {
    Core::instance.seat->getCursor().warp(boxGetCenterCoords(getLayoutGeometry()).into<double>());
    Core::instance.seat->getInput().rebasePointer();
}

void Output::arrangeLayers() {
    wlr_box fullArea   = getLayoutGeometry();
    wlr_box usableArea = fullArea;

    for (auto& list : layers) {
        Layer* layer;
        wl_list_for_each(layer, &list, link) {
            layer->configure(fullArea, usableArea);
        }
    }

    m_usableArea = usableArea;
}

NAMESPACE_SYCAMORE_END