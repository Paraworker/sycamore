#ifndef SYCAMORE_XDG_VIEW_H
#define SYCAMORE_XDG_VIEW_H

#include "sycamore/defines.h"
#include "sycamore/desktop/View.h"
#include "sycamore/utils/Listener.h"

NAMESPACE_SYCAMORE_BEGIN

class XdgView final : public View {
public:
    /**
     * @brief Create XdgView
     * @return nullptr on failure
     */
    static XdgView* create(wlr_xdg_toplevel* toplevel);

    Role role() const override { return XDG; }

    uint32_t setMaximized(bool maximized) override;

    uint32_t setFullscreen(bool fullscreen) override;

    uint32_t setActivated(bool activated) override;

    uint32_t setResizing(bool resizing) override;

    uint32_t setSize(uint32_t width, uint32_t height) override;

    wlr_box getGeometry() override;

    void close() override;

    XdgView(const XdgView&) = delete;
    XdgView(XdgView&&) = delete;
    XdgView& operator=(const XdgView&) = delete;
    XdgView& operator=(XdgView&&) = delete;

private:
    XdgView(wlr_xdg_toplevel* toplevel, wlr_scene_tree* tree);

    ~XdgView() override;

private:
    wlr_xdg_toplevel* m_toplevel;
    wlr_box           m_committedGeometry{0, 0, 0, 0};

private:
    Listener m_destroy;
    Listener m_map;
    Listener m_unmap;
    Listener m_commit;
    Listener m_newPopup;
    Listener m_move;
    Listener m_resize;
    Listener m_fullscreen;
    Listener m_maximize;
    Listener m_minimize;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_XDG_VIEW_H