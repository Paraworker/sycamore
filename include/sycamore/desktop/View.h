#ifndef SYCAMORE_VIEW_H
#define SYCAMORE_VIEW_H

#include "sycamore/defines.h"
#include "sycamore/output/Output.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <memory>

NAMESPACE_SYCAMORE_BEGIN

class Output;

class View {
public:
    enum Role { XDG, XWAYLAND };

    struct State {
        bool maximized  = false;
        bool fullscreen = false;
    };

    struct RestoreData {
        wlr_box maximize;
        wlr_box fullscreen;
    };

    struct Events {
        wl_signal map;
        wl_signal unmap;
        wl_signal destroy;
    };

public:
    Output* getOutput() const;

    void setToOutputCenter(Output* output);

    void moveTo(const Point<int32_t>& position) const {
        wlr_scene_node_set_position(&m_tree->node, position.x, position.y);
    }

    Point<int32_t> getPosition() const {
        return {m_tree->node.x, m_tree->node.y};
    }

    void toFront() const {
        wlr_scene_node_raise_to_top(&m_tree->node);
    }

    auto getBaseSurface() const { return m_surface; }

    bool isMapped() const { return m_surface->mapped; }

    bool isPinned() const { return m_state.maximized || m_state.fullscreen; }

    State& state() { return m_state; }

    virtual Role role() const = 0;

    virtual uint32_t setMaximized(bool maximized) = 0;

    virtual uint32_t setFullscreen(bool fullscreen) = 0;

    virtual uint32_t setActivated(bool activated) = 0;

    virtual uint32_t setResizing(bool resizing) = 0;

    virtual uint32_t setSize(uint32_t width, uint32_t height) = 0;

    virtual wlr_box getGeometry() = 0;

    virtual void close() = 0;

public:
    Events      events;
    RestoreData restore;
    wl_list     link{}; // ShellManager::mappedViewList

protected:
    View(wlr_surface* surface, wlr_scene_tree* tree);
    virtual ~View();

protected:
    wlr_surface*    m_surface;
    wlr_scene_tree* m_tree;
    State           m_state;
};

class ViewElement final : public SceneElement {
public:
    View* getView() const { return m_view; }

private:
    ViewElement(wlr_scene_node* node, View* view)
        : SceneElement(SceneElement::VIEW, node), m_view(view) {}

    ~ViewElement() override = default;

private:
    friend class View;
    View* m_view;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_VIEW_H