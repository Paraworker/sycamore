#include "sycamore/output/OutputLayout.h"

#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace sycamore
{

OutputLayout* OutputLayout::create(wl_display* display)
{
    // Will be destroyed by listener
    return new OutputLayout{display};
}

OutputLayout::OutputLayout(wl_display* display)
    : m_handle{nullptr}
    , m_outputCount{0}
{
    if (m_handle = wlr_output_layout_create(display); !m_handle)
    {
        throw std::runtime_error("Create wlr_output_layout failed!");
    }

    m_destroy
    .connect(m_handle->events.destroy)
    .set([this](auto)
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