#ifndef SYCAMORE_POPUP_H
#define SYCAMORE_POPUP_H

#include "sycamore/defines.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/utils/Listener.h"
#include "sycamore/wlroots.h"

NAMESPACE_SYCAMORE_BEGIN

class Popup {
public:
    static void onCreate(wlr_xdg_popup* handle, wlr_scene_tree* parent);

public:
    Popup(const Popup&) = delete;
    Popup(Popup&&) = delete;
    Popup& operator=(const Popup&) = delete;
    Popup& operator=(Popup&&) = delete;

private:
    Popup(wlr_xdg_popup* handle, wlr_scene_tree* tree);
    ~Popup();

private:
    wlr_xdg_popup*  m_handle;
    wlr_scene_tree* m_tree;

private:
    Listener m_newPopup;
    Listener m_destroy;
};

class PopupElement final : public SceneElement {
public:
    Popup* getPopup() const { return m_popup; }

private:
    PopupElement(wlr_scene_node* node, Popup* popup)
        : SceneElement(SceneElement::POPUP, node), m_popup(popup) {}

    ~PopupElement() override = default;

private:
    friend class Popup;
    Popup* m_popup;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_POPUP_H