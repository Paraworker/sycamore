#include "sycamore/output/OutputLayout.h"
#include "sycamore/output/Output.h"
#include "sycamore/Core.h"

#include <spdlog/spdlog.h>

NAMESPACE_SYCAMORE_BEGIN

OutputLayout::UPtr OutputLayout::create() {
    auto layout = wlr_output_layout_create();
    if (!layout) {
        spdlog::error("Create wlr_output_layout failed");
        return nullptr;
    }

    return UPtr{new OutputLayout{layout}};
}

OutputLayout::OutputLayout(wlr_output_layout* handle) : m_handle(handle) {}

OutputLayout::~OutputLayout() {
    wlr_output_layout_destroy(m_handle);
}

bool OutputLayout::add(Output* output) {
    auto layoutOutput = wlr_output_layout_add_auto(m_handle, output->getHandle());
    if (!layoutOutput) {
        spdlog::error("Output: {} add to wlr_output_layout failed", output->name());
        return false;
    }

    wlr_scene_output_layout_add_output(Core::instance.scene->getLayout(), layoutOutput, output->getSceneOutput());
    m_outputList.add(output->link);

    return true;
}

void OutputLayout::remove(Output* output) {
    m_outputList.remove(output->link);
    wlr_output_layout_remove(m_handle, output->getHandle());
}

Output* OutputLayout::findOutputAt(const Point<double>& coords) const {
    auto output = wlr_output_layout_output_at(m_handle, coords.x, coords.y);
    if (!output) {
        return nullptr;
    }

    return static_cast<Output*>(output->data);
}

NAMESPACE_SYCAMORE_END