#ifndef SYCAMORE_POPUP_H
#define SYCAMORE_POPUP_H

#include "sycamore/scene/Element.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>

namespace sycamore
{

class Popup
{
public:
    struct Handler
    {
        /**
         * @brief Destructor
         */
        virtual ~Handler() = default;

        /**
         * @brief Unconstrain popup
         */
        virtual void unconstrain(Popup& popup) = 0;
    };

public:
    /**
     * @brief Constructor
     */
    Popup(wlr_xdg_popup* handle, wlr_scene_tree* parent, std::shared_ptr<Handler> handler);

    /**
     * @brief Destructor
     */
    ~Popup();

    void unconstrainFromBox(const wlr_box& box)
    {
        wlr_xdg_popup_unconstrain_from_box(m_handle, &box);
    }

    Popup(const Popup&) = delete;
    Popup(Popup&&) = delete;
    Popup& operator=(const Popup&) = delete;
    Popup& operator=(Popup&&) = delete;

private:
    wlr_xdg_popup*           m_handle;
    wlr_scene_tree*          m_tree;
    std::shared_ptr<Handler> m_handler;

    Listener                 m_surfaceCommit;
    Listener                 m_reposition;
    Listener                 m_newPopup;
    Listener                 m_destroy;
};

struct PopupElement final : scene::Element
{
    Popup&   popup;
    Listener destroy;

    PopupElement(wlr_scene_node& node, Popup& popup)
        : Element{POPUP}, popup{popup}
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

#endif //SYCAMORE_POPUP_H