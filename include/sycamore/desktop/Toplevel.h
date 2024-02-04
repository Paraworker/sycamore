#ifndef SYCAMORE_TOPLEVEL_H
#define SYCAMORE_TOPLEVEL_H

#include "sycamore/scene/Element.h"
#include "sycamore/utils/Point.h"
#include "sycamore/wlroots.h"

#include <list>

namespace sycamore
{

class Output;

class Toplevel
{
public:
    enum Role { XDG, XWAYLAND };

    struct State
    {
        bool maximized  = false;
        bool fullscreen = false;
    };

    struct RestoreData
    {
        wlr_box maximize;
        wlr_box fullscreen;
    };

    struct Events
    {
        wl_signal map;
        wl_signal unmap;
    };

    using Iter = std::list<Toplevel*>::iterator;

public:
    Output* getOutput() const;

    void setToOutputCenter(const Output& output);

    void moveTo(const Point<int32_t>& position) const
    {
        wlr_scene_node_set_position(&m_tree->node, position.x, position.y);
    }

    Point<int32_t> getPosition() const
    {
        return {m_tree->node.x, m_tree->node.y};
    }

    void toFront() const
    {
        wlr_scene_node_raise_to_top(&m_tree->node);
    }

    auto getBaseSurface() const
    {
        return m_surface;
    }

    bool isMapped() const
    {
        return m_surface->mapped;
    }

    bool isPinned() const
    {
        return m_state.maximized || m_state.fullscreen;
    }

    State& state()
    {
        return m_state;
    }

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
    Iter        iter;

protected:
    /**
     * @brief Constructor
     */
    Toplevel(wlr_surface* surface, wlr_scene_tree* tree);

    /**
     * @brief Destructor
     */
    virtual ~Toplevel();

protected:
    wlr_surface*    m_surface;
    wlr_scene_tree* m_tree;
    State           m_state;
};

struct ToplevelElement final : scene::Element
{
    Toplevel& toplevel;

    ToplevelElement(wlr_scene_node* node, Toplevel& toplevel)
        : Element{TOPLEVEL, node}
        , toplevel{toplevel}
    {}

    ~ToplevelElement() override = default;
};

}

#endif //SYCAMORE_TOPLEVEL_H