#include "sycamore/desktop/View.h"
#include "sycamore/utils/box.h"
#include "sycamore/Core.h"

NAMESPACE_SYCAMORE_BEGIN

View::View(wlr_surface* surface, wlr_scene_tree* tree)
    : m_surface(surface), m_tree(tree)
{
    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);
    wl_signal_init(&events.destroy);

    // Create SceneElement
    new ViewElement{&m_tree->node, this};
}

View::~View() = default;

Output* View::getOutput() const
{
    // FIXME
    return Core::instance.seat->getCursor().atOutput();
}

void View::setToOutputCenter(Output* output)
{
    if (!output)
    {
        return;
    }

    auto outputBox  = output->getLayoutGeometry();
    auto center     = boxGetCenterCoords(outputBox);
    auto viewGeoBox = getGeometry();

    viewGeoBox.x = center.x - (viewGeoBox.width / 2);
    viewGeoBox.y = center.y - (viewGeoBox.height / 2);

    // Don't let view's top being out of output.
    if (viewGeoBox.y < outputBox.y)
    {
        viewGeoBox.y = outputBox.y;
    }

    moveTo({viewGeoBox.x, viewGeoBox.y});
}

NAMESPACE_SYCAMORE_END