#ifndef SYCAMORE_POPUP_H
#define SYCAMORE_POPUP_H

#include "sycamore/desktop/Layer.h"
#include "sycamore/desktop/Toplevel.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

namespace sycamore
{

class Popup
{
public:
    // Operations for different toplevel parent
    class OwnerHandler
    {
    public:
        using SPtr = std::shared_ptr<OwnerHandler>;

    public:
        /**
         * @brief Destructor
         */
        virtual ~OwnerHandler() = default;

        /**
         * @brief Unconstrain
         */
        virtual void unconstrain(Popup& popup) = 0;
    };

    class ToplevelHandler : public OwnerHandler
    {
    public:
        explicit ToplevelHandler(Toplevel* toplevel) : m_toplevel{toplevel} {}

        ~ToplevelHandler() override = default;

        void unconstrain(Popup& popup) override
        {
            if (auto output = m_toplevel->getOutput(); output)
            {
                auto geo = m_toplevel->getGeometry();
                auto pos = m_toplevel->getPosition();
                auto box = output->getRelativeGeometry();

                box.x = -pos.x + geo.x;
                box.y = -pos.y + geo.y;

                popup.unconstrainFromBox(box);
            }
        }

    private:
        Toplevel* m_toplevel;
    };

    class LayerHandler : public OwnerHandler
    {
    public:
        explicit LayerHandler(Layer* layer) : m_layer{layer} {}

        ~LayerHandler() override = default;

        void unconstrain(Popup& popup) override
        {
            auto pos = m_layer->getPosition();
            auto box = m_layer->getOutput()->getRelativeGeometry();

            box.x = -pos.x;
            box.y = -pos.y;

            popup.unconstrainFromBox(box);
        }

    private:
        Layer* m_layer;
    };

public:
    /**
     * @brief Create Popup
     * @return nullptr on failure
     */
    static Popup* create(wlr_xdg_popup* handle, wlr_scene_tree* parentTree, const OwnerHandler::SPtr& owner);

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
    Popup(wlr_xdg_popup* handle, wlr_scene_tree* tree, OwnerHandler::SPtr owner);

    /**
     * @brief Destructor
     */
    ~Popup();

private:
    wlr_xdg_popup*     m_handle;
    wlr_scene_tree*    m_tree;
    OwnerHandler::SPtr m_owner;

private:
    Listener m_surfaceCommit;
    Listener m_reposition;
    Listener m_newPopup;
    Listener m_destroy;
};

class PopupElement final : public SceneElement
{
public:
    PopupElement(wlr_scene_node* node, Popup* popup)
        : SceneElement{SceneElement::POPUP, node}
        , m_popup{popup} {}

    ~PopupElement() override = default;

    Popup& getPopup() const
    {
        return *m_popup;
    }

private:
    Popup* m_popup;
};

}

#endif //SYCAMORE_POPUP_H