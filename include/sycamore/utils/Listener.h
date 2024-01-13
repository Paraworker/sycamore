#ifndef SYCAMORE_LISTENER_H
#define SYCAMORE_LISTENER_H

#include <cassert>
#include <functional>
#include <wayland-server-core.h>

namespace sycamore
{

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
     */
    ~Listener()
    {
        if (isConnected())
        {
            wl_list_remove(&m_wrapped.link);
        }
    }

    /**
     * @brief Set callback
     */
    template<typename Fn>
    void set(Fn&& callback)
    {
        m_callback = std::forward<Fn>(callback);
    }

    /**
     * @brief Connect to signal
     */
    void connect(wl_signal& signal)
    {
        assert(!isConnected());
        wl_signal_add(&signal, &m_wrapped);
    }

    /**
     * @brief Disconnect form signal
     */
    void disconnect()
    {
        assert(isConnected());
        wl_list_remove(&m_wrapped.link);
        wl_list_init(&m_wrapped.link);
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

}

#endif //SYCAMORE_LISTENER_H