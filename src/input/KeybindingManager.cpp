#include "sycamore/input/KeybindingManager.h"

#include "sycamore/desktop/WindowManager.h"
#include "sycamore/utils/process.h"
#include "sycamore/Core.h"

namespace sycamore
{

KeybindingManager::KeybindingManager()
{
    add({WLR_MODIFIER_LOGO, XKB_KEY_d}, [] { spawn("fuzzel -i Papirus"); });
    add({WLR_MODIFIER_LOGO, XKB_KEY_Return}, [] { spawn("gnome-terminal"); });
    add({WLR_MODIFIER_LOGO, XKB_KEY_q}, [] { windowManager.closeFocusedToplevel(); });
    add({WLR_MODIFIER_LOGO, XKB_KEY_Tab}, [] { windowManager.cycleToplevel(); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_Escape}, [] { core.terminate(); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_1}, [] { core.switchVt(1); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_2}, [] { core.switchVt(2); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_3}, [] { core.switchVt(3); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_4}, [] { core.switchVt(4); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_5}, [] { core.switchVt(5); });
    add({WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_6}, [] { core.switchVt(6); });
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