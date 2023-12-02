#ifndef SYCAMORE_LISTENER_H
#define SYCAMORE_LISTENER_H

#include "sycamore/defines.h"

#include <cassert>
#include <functional>
#include <wayland-server-core.h>

NAMESPACE_SYCAMORE_BEGIN

// A wrapper for wl_listener
class Listener
{
public:
    /**
     * @brief Constructor
     */
    Listener() : m_wrapped{{}, onSignal}
    {
        wl_list_init(&m_wrapped.link);
    }

    /**
     * @brief Destructor
     * @note Disconnect automatically
     */
    ~Listener()
    {
        disconnect();
    }

    /**
     * @brief Set callback
     */
    template<typename Func>
    Listener& set(Func&& callback)
    {
        m_callback = std::forward<Func>(callback);
        return *this;
    }

    /**
     * @brief Connect to signal
     */
    Listener& connect(wl_signal& signal)
    {
        assert(!isConnected() && "connect() on a connected listener!");
        wl_signal_add(&signal, &m_wrapped);
        return *this;
    }

    /**
     * @brief Disconnect form signal
     * @note No-op if signal isn't connected
     */
    Listener& disconnect()
    {
        if (isConnected())
        {
            wl_list_remove(&m_wrapped.link);
            wl_list_init(&m_wrapped.link);
        }

        return *this;
    }

    /**
     * @brief Is signal connected
     */
    bool isConnected() const
    {
        return !wl_list_empty(&m_wrapped.link);
    }

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

private:
    static void onSignal(wl_listener* listener, void* data)
    {
        reinterpret_cast<Listener*>(listener)->m_callback(data);
    }

private:
    using Callback = std::function<void(void*)>;

    wl_listener m_wrapped;
    Callback    m_callback;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_LISTENER_H