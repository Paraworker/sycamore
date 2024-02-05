#include "sycamore/output/OutputManager.h"

#include "sycamore/Core.h"
#include <spdlog/spdlog.h>

namespace sycamore
{

void OutputManager::newOutput(wlr_output* handle)
{
    spdlog::info("New Output: {}", handle->name);

    if (!wlr_output_init_render(handle, core.allocator, core.renderer))
    {
        spdlog::error("Output: {} init render failed", handle->name);
        wlr_output_destroy(handle);
        return;
    }

    // Add to output layout
    auto layoutOutput = wlr_output_layout_add_auto(core.outputLayout, handle);
    if (!layoutOutput)
    {
        throw std::runtime_error{"Output add to wlr_output_layout failed!"};
    }

    // Add to scene layout
    auto sceneOutput = core.scene.addOutput(handle, layoutOutput);

    const auto output = m_outputList.emplace(m_outputList.end(), handle, sceneOutput);
    output->iter = output;

    // TODO: configurable
    output->apply();
}

void OutputManager::destroyOutput(Output* output)
{
    wlr_output_layout_remove(core.outputLayout, output->getHandle());
    m_outputList.erase(output->iter);
}

size_t OutputManager::outputCount() const
{
    return m_outputList.size();
}

Output* OutputManager::findOutputAt(const Point<double>& coords)
{
    auto output = wlr_output_layout_output_at(core.outputLayout, coords.x, coords.y);
    if (!output)
    {
        return {};
    }

    return static_cast<Output*>(output->data);
}

}