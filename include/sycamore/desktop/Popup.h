#ifndef SYCAMORE_POPUP_H
#define SYCAMORE_POPUP_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/output/Output.h"
#include "sycamore/scene/Element.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

#include <memory>

namespace sycamore
{

class Popup
{
public:
    // Operations for different toplevel parent
    struct OwnerHandler
    {
        /**
         * @brief Destructor
         */
        virtual ~OwnerHandler() = default;

        /**
         * @brief Unconstrain
         */
        virtual void unconstrain(Popup& popup) = 0;
    };

    struct ToplevelHandler : OwnerHandler
    {
        Toplevel& toplevel;

        explicit ToplevelHandler(Toplevel& toplevel)
            : toplevel{toplevel} {}

        ~ToplevelHandler() override = default;

        void unconstrain(Popup& popup) override
        {
            if (auto output = toplevel.getOutput(); output)
            {
                auto geo = toplevel.getGeometry();
                auto pos = toplevel.getPosition();
                auto box = output->getRelativeGeometry();

                box.x = -pos.x + geo.x;
                box.y = -pos.y + geo.y;

                popup.unconstrainFromBox(box);
            }
        }
    };

    struct LayerHandler : OwnerHandler
    {
        Layer& layer;

        explicit LayerHandler(Layer& layer)
            : layer{layer} {}

        ~LayerHandler() override = default;

        void unconstrain(Popup& popup) override
        {
            auto pos = layer.getPosition();
            auto box = layer.getOutput()->getRelativeGeometry();

            box.x = -pos.x;
            box.y = -pos.y;

            popup.unconstrainFromBox(box);
        }
    };

public:
    template<typename... Args>
    static void create(Args&&... args)
    {
        new Popup{std::forward<Args>(args)...};
    }

    void unconstrainFromBox(const wlr_box& box)
    {
        wlr_xdg_popup_unconstrain_from_box(m_handle, &box);
    }

    Popup(const Popup&) = delete;
    Popup(Popup&&) = delete;
    Popup& operator=(const Popup&) = delete;
    Popup& operator=(Popup&&) = delete;

private:
    /**
     * @brief Constructor
     */
    Popup(wlr_xdg_popup* handle, wlr_scene_tree* parent, std::shared_ptr<OwnerHandler> owner);

    /**
     * @brief Destructor
     */
    ~Popup();

private:
    wlr_xdg_popup*                m_handle;
    wlr_scene_tree*               m_tree;
    std::shared_ptr<OwnerHandler> m_owner;

    Listener                      m_surfaceCommit;
    Listener                      m_reposition;
    Listener                      m_newPopup;
    Listener                      m_destroy;
};

struct PopupElement final : scene::Element
{
    Popup& popup;

    PopupElement(wlr_scene_node* node, Popup& popup)
        : Element{POPUP, node}
        , popup{popup}
    {}

    ~PopupElement() override = default;
};

}

#endif //SYCAMORE_POPUP_H