#include "sycamore/output/OutputLayout.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

OutputLayout* OutputLayout::create(wl_display* display)
{
    auto handle = wlr_output_layout_create(display);
    if (!handle)
    {
        spdlog::error("Create wlr_output_layout failed");
        return {};
    }

    return new OutputLayout{handle};
}

OutputLayout::OutputLayout(wlr_output_layout* handle)
    : m_handle{handle}
    , m_outputCount{0}
{
    m_destroy
    .connect(handle->events.destroy)
    .set([this](void*)
    {
        delete this;
    });
}

OutputLayout::~OutputLayout() = default;

bool OutputLayout::addAuto(Output* output)
{
    auto layoutOutput = wlr_output_layout_add_auto(m_handle, output->getHandle());
    if (!layoutOutput)
    {
        spdlog::error("Output: {} add to wlr_output_layout failed", output->name());
        return false;
    }

    wlr_scene_output_layout_add_output(Core::instance.scene->getLayout(), layoutOutput, output->getSceneOutput());
    ++m_outputCount;

    return true;
}

void OutputLayout::remove(Output* output)
{
    wlr_output_layout_remove(m_handle, output->getHandle());
    --m_outputCount;
}

Output* OutputLayout::findOutputAt(const Point<double>& coords) const
{
    auto output = wlr_output_layout_output_at(m_handle, coords.x, coords.y);
    if (!output)
    {
        return {};
    }

    return static_cast<Output*>(output->data);
}

NAMESPACE_SYCAMORE_END