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
    enum Kind { XDG, XWAYLAND };

    struct State
    {
        bool maximized  = false;
        bool fullscreen = false;
    };

    struct Restore
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
    /**
     * @brief Constructor
     */
    Toplevel(wlr_surface* surface, wlr_scene_tree* tree);

    /**
     * @brief Destructor
     */
    virtual ~Toplevel();

    Output* output() const;

    void setToOutputCenter(const Output& output);

    void moveTo(const Point<int32_t>& position) const
    {
        wlr_scene_node_set_position(&m_tree->node, position.x, position.y);
    }

    Point<int32_t> position() const
    {
        return {m_tree->node.x, m_tree->node.y};
    }

    void toFront() const
    {
        wlr_scene_node_raise_to_top(&m_tree->node);
    }

    auto baseSurface() const
    {
        return m_surface;
    }

    bool isMapped() const
    {
        return m_surface->mapped;
    }

    bool isPinned() const
    {
        return state.maximized || state.fullscreen;
    }

    virtual Kind kind() const = 0;

    virtual uint32_t setMaximized(bool state) = 0;

    virtual uint32_t setFullscreen(bool state) = 0;

    virtual uint32_t setActivated(bool state) = 0;

    virtual uint32_t setResizing(bool state) = 0;

    virtual uint32_t setSize(uint32_t width, uint32_t height) = 0;

    virtual wlr_box geometry() = 0;

    virtual void close() = 0;

public:
    Events  events;
    State   state;
    Restore restore;
    Iter    iter;

protected:
    wlr_surface*    m_surface;
    wlr_scene_tree* m_tree;
};

struct ToplevelElement final : scene::Element
{
    Toplevel& toplevel;
    Listener  destroy;

    ToplevelElement(wlr_scene_node& node, Toplevel& toplevel)
        : Element{TOPLEVEL}, toplevel{toplevel}
    {
        // attach node
        node.data = this;

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(node.events.destroy);
    }
};

}

#endif //SYCAMORE_TOPLEVEL_H