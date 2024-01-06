#include "sycamore/input/KeybindingManager.h"

#include "sycamore/input/keybindingDispatchers.h"

namespace sycamore
{

KeybindingManager KeybindingManager::instance{};

KeybindingManager::KeybindingManager()
{
    add({WLR_MODIFIER_LOGO, XKB_KEY_d}, Spawn{"fuzzel -i Papirus"});
    add({WLR_MODIFIER_LOGO, XKB_KEY_Return}, Spawn{"gnome-terminal"});
    add({WLR_MODIFIER_LOGO, XKB_KEY_q}, CloseFocusedToplevel{});
    add({WLR_MODIFIER_LOGO, XKB_KEY_Tab}, CycleToplevel{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_Escape}, Terminate{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_1}, SwitchVT<1>{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_2}, SwitchVT<2>{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_3}, SwitchVT<3>{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_4}, SwitchVT<4>{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_5}, SwitchVT<5>{});
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_6}, SwitchVT<6>{});
}

KeybindingManager::~KeybindingManager() = default;

bool KeybindingManager::add(const KeyInfo& info, Dispatcher&& dispatcher)
{
    if (!info)
    {
        return false;
    }

    return m_bindingMap.emplace(info, std::move(dispatcher)).second;
}

bool KeybindingManager::remove(const KeyInfo& info)
{
    return (m_bindingMap.erase(info) == 1);
}

bool KeybindingManager::dispatch(const KeyInfo& info)
{
    if (!info)
    {
        return false;
    }

    auto itr = m_bindingMap.find(info);
    if (itr == m_bindingMap.end())
    {
        return false;
    }

    itr->second();

    return true;
}

}