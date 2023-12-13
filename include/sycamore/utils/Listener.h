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
    Listener() noexcept : m_wrapped{{}, onSignal}
    {
        wl_list_init(&m_wrapped.link);
    }

    /**
     * @brief Destructor
     * @note Disconnect automatically
     */
    ~Listener() noexcept
    {
        disconnect();
    }

    /**
     * @brief Set callback
     */
    template<typename Fn>
    Listener& set(Fn&& callback) noexcept
    {
        m_callback = std::forward<Fn>(callback);
        return *this;
    }

    /**
     * @brief Connect to signal
     */
    Listener& connect(wl_signal& signal) noexcept
    {
        assert(!isConnected() && "connect() on a connected listener!");
        wl_signal_add(&signal, &m_wrapped);
        return *this;
    }

    /**
     * @brief Disconnect form signal
     * @note No-op if signal isn't connected
     */
    Listener& disconnect() noexcept
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
    bool isConnected() const noexcept
    {
        return !wl_list_empty(&m_wrapped.link);
    }

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

private:
    static void onSignal(wl_listener* listener, void* data) noexcept
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