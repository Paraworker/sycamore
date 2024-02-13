#ifndef SYCAMORE_XDG_TOPLEVEL_H
#define SYCAMORE_XDG_TOPLEVEL_H

#include "sycamore/desktop/Toplevel.h"
#include "sycamore/utils/Listener.h"

namespace sycamore
{

class XdgToplevel final : public Toplevel
{
public:
    explicit XdgToplevel(wlr_xdg_toplevel* toplevel);

    ~XdgToplevel() override;

    Kind kind() const override
    {
        return XDG;
    }

    uint32_t setMaximized(bool state) override;

    uint32_t setFullscreen(bool state) override;

    uint32_t setActivated(bool state) override;

    uint32_t setResizing(bool state) override;

    uint32_t setSize(uint32_t width, uint32_t height) override;

    wlr_box geometry() override;

    void close() override;

    XdgToplevel(const XdgToplevel&) = delete;
    XdgToplevel(XdgToplevel&&) = delete;
    XdgToplevel& operator=(const XdgToplevel&) = delete;
    XdgToplevel& operator=(XdgToplevel&&) = delete;

private:
    wlr_xdg_toplevel* m_toplevel;
    wlr_box           m_lastGeo{0, 0, 0, 0};

    Listener          m_map;
    Listener          m_unmap;
    Listener          m_commit;
    Listener          m_destroy;

    Listener          m_newPopup;
    Listener          m_move;
    Listener          m_resize;
    Listener          m_fullscreen;
    Listener          m_maximize;
    Listener          m_minimize;
};

}

#endif //SYCAMORE_XDG_TOPLEVEL_H