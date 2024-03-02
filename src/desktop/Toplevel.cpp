#include "sycamore/desktop/Toplevel.h"

#include "sycamore/input/Seat.h"
#include "sycamore/output/Output.h"
#include "sycamore/utils/box_helper.h"
#include "sycamore/Core.h"

namespace sycamore
{

Toplevel::Toplevel(wlr_surface* surface, wlr_scene_tree* tree)
    : m_surface{surface}
    , m_tree{tree}
{
    new ToplevelElement{m_tree->node, *this};

    wl_signal_init(&events.map);
    wl_signal_init(&events.unmap);
}

Toplevel::~Toplevel() = default;

Output* Toplevel::output() const
{
    // FIXME
    return core.cursor.atOutput();
}

void Toplevel::setToOutputCenter(const Output& output)
{
    auto outputGeo   = output.layoutGeometry();
    auto center      = boxGetCenter(outputGeo);
    auto toplevelGeo = geometry();

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