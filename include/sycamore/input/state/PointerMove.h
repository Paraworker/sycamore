#ifndef SYCAMORE_POINTER_MOVE_H
#define SYCAMORE_POINTER_MOVE_H

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/utils/Listener.h"
#include "InputState.h"

namespace sycamore
{

class PointerMove final : public InputState
{
public:
    PointerMove(Toplevel* toplevel);

    ~PointerMove() override;

    void onEnable() override;

    void onDisable() override;

    void onPointerButton(wlr_pointer_button_event* event) override;

    void onPointerMotion(uint32_t timeMsec) override;

    bool isInteractive() const override;

private:
    Toplevel*     m_toplevel;
    Listener      m_toplevelUnmap;
    Point<double> m_delta;
};

}

#endif //SYCAMORE_POINTER_MOVE_H