#ifndef SYCAMORE_POINTER_RESIZE_H
#define SYCAMORE_POINTER_RESIZE_H

#include "sycamore/desktop/Toplevel.h"
#include "InputState.h"

namespace sycamore
{

class PointerResize : public InputState
{
public:
    PointerResize(Toplevel* toplevel, uint32_t edges);

    ~PointerResize() override;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    bool isInteractive() const override;

private:
    Toplevel*     m_toplevel;
    Listener      m_toplevelUnmap;
    uint32_t      m_edges;
    wlr_box       m_grabGeo;
    Point<double> m_delta;
};

}

#endif //SYCAMORE_POINTER_RESIZE_H