#include "sycamore/output/OutputLayout.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <stdexcept>
#include <spdlog/spdlog.h>

namespace sycamore
{

OutputLayout* OutputLayout::create(wl_display* display)
{
    return new OutputLayout{display};
}

OutputLayout::OutputLayout(wl_display* display)
    : m_handle{wlr_output_layout_create(display)}
    , m_outputCount{0}
{
    if (!m_handle)
    {
        throw std::runtime_error("Create wlr_output_layout failed!");
    }

    m_destroy
    .connect(m_handle->events.destroy)
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

}