#include "sycamore/desktop/Toplevel.h"
#include "sycamore/utils/box_helper.h"
#include "sycamore/Core.h"

namespace sycamore
{

Toplevel::Toplevel(wlr_surface* surface, wlr_scene_tree* tree)
    : m_surface{surface}, m_tree{tree}
{
    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);

    // Create SceneElement
    new ToplevelElement{&m_tree->node, this};
}

Toplevel::~Toplevel() = default;

Output* Toplevel::getOutput() const
{
    // FIXME
    return Core::instance.seat->cursor.atOutput();
}

void Toplevel::setToOutputCenter(Output* output)
{
    if (!output)
    {
        return;
    }

    auto outputGeo   = output->getLayoutGeometry();
    auto center      = boxGetCenterCoords(outputGeo);
    auto toplevelGeo = getGeometry();

    toplevelGeo.x = center.x - (toplevelGeo.width / 2);
    toplevelGeo.y = center.y - (toplevelGeo.height / 2);

    // Don't let top edge being out of output.
    if (toplevelGeo.y < outputGeo.y)
    {
        toplevelGeo.y = outputGeo.y;
    }

    moveTo({toplevelGeo.x, toplevelGeo.y});
}

}