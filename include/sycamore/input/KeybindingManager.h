#ifndef SYCAMORE_KEYBINDING_MANAGER_H
#define SYCAMORE_KEYBINDING_MANAGER_H

#include <functional>
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

namespace sycamore
{

struct KeyInfo
{
    uint32_t     modifiers;
    xkb_keysym_t sym;

    bool operator==(const KeyInfo& other) const
    {
        return (modifiers == other.modifiers) && (sym == other.sym);
    }

    explicit operator bool() const { return modifiers != 0; }

    struct Hash
    {
        std::size_t operator()(const KeyInfo& info) const noexcept
        {
            return std::hash<uint32_t>()(info.modifiers) ^ std::hash<xkb_keysym_t>()(info.sym);
        }
    };
};

class KeybindingManager
{
public:
    using Dispatcher = std::function<void(void)>;

public:
    bool add(const KeyInfo& info, Dispatcher&& dispatcher);

    bool remove(const KeyInfo& info);

    bool dispatch(const KeyInfo& info);

public:
    static KeybindingManager instance;

private:
    /**
     * @brief Constructor
     */
    KeybindingManager();

    /**
     * @brief Destructor
     */
    ~KeybindingManager();

private:
    std::unordered_map<KeyInfo, Dispatcher, KeyInfo::Hash> m_bindingMap;
};

}

#endif //SYCAMORE_KEYBINDING_MANAGER_H