#include "sycamore/output/Output.h"

#include "sycamore/desktop/Layer.h"
#include "sycamore/input/Seat.h"
#include "sycamore/output/OutputManager.h"
#include "sycamore/utils/box_helper.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

namespace sycamore
{

Output::Output(wlr_output* handle, wlr_scene_output* sceneOutput)
    : m_handle{handle}
    , m_sceneOutput{sceneOutput}
    , m_usableArea{}
{
    handle->data = this;

    wl_signal_init(&events.destroy);

    m_frame = [this](auto)
    {
        // Render the scene if needed and commit the output
        wlr_scene_output_commit(m_sceneOutput, nullptr);

        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        wlr_scene_output_send_frame_done(m_sceneOutput, &now);
    };
    m_frame.connect(handle->events.frame);

    m_requestState = [this](void* data)
    {
        wlr_output_commit_state(m_handle, static_cast<wlr_output_event_request_state*>(data)->state);
    };
    m_requestState.connect(handle->events.request_state);

    m_destroy = [this](auto)
    {
        outputManager.removeOutput(this);
    };
    m_destroy.connect(handle->events.destroy);
}

Output::~Output()
{
    m_handle->data = nullptr;
}

bool Output::apply()
{
    wlr_output_state state{};
    wlr_output_state_init(&state);

    wlr_output_state_set_enabled(&state, true);

    if (auto mode = wlr_output_preferred_mode(m_handle); mode)
    {
        wlr_output_state_set_mode(&state, mode);
    }

    if (!wlr_output_commit_state(m_handle, &state))
    {
        spdlog::error("Output: {} commit state failed", m_handle->name);
        wlr_output_state_finish(&state);
        return false;
    }

    wlr_output_state_finish(&state);

    // Center cursor if this is the first output
    if (outputManager.outputCount() == 1)
    {
        ensureCursor();
    }

    arrangeLayers();

    return true;
}

wlr_box Output::relativeGeometry() const
{
    wlr_box box{};
    wlr_output_effective_resolution(m_handle, &box.width, &box.height);
    return box;
}

wlr_box Output::layoutGeometry() const
{
    wlr_box box{};
    wlr_output_effective_resolution(m_handle, &box.width, &box.height);

    auto layoutOutput = wlr_output_layout_get(core.outputLayout, m_handle);
    if (!layoutOutput)
    {
        return box;
    }

    box.x = layoutOutput->x;
    box.y = layoutOutput->y;

    return box;
}

void Output::ensureCursor() const
{
    core.cursor.warp(static_cast<Point<double>>(boxGetCenter(layoutGeometry())));
    core.seat->input->rebasePointer();
}

void Output::arrangeLayers()
{
    auto fullArea   = layoutGeometry();
    auto usableArea = fullArea;

    for (auto& list : layerList)
    {
        for (auto layer : list)
        {
            layer->configure(fullArea, usableArea);
        }
    }

    m_usableArea = usableArea;
}

}