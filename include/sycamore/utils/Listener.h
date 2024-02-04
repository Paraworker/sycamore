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
    Listener() : m_wrapper{{{}, onSignal}, {}}
    {
        wl_list_init(&m_wrapper.listener.link);
    }

    /**
     * @brief Destructor
     */
    ~Listener()
    {
        if (isConnected())
        {
            wl_list_remove(&m_wrapper.listener.link);
        }
    }

    /**
     * @brief Set callback
     */
    template<typename Fn>
    void notify(Fn&& callback)
    {
        m_wrapper.callback = std::forward<Fn>(callback);
    }

    /**
     * @brief Connect to signal
     */
    void connect(wl_signal& signal)
    {
        assert(!isConnected());
        wl_signal_add(&signal, &m_wrapper.listener);
    }

    /**
     * @brief Disconnect form signal
     */
    void disconnect()
    {
        assert(isConnected());
        wl_list_remove(&m_wrapper.listener.link);
        wl_list_init(&m_wrapper.listener.link);
    }

    /**
     * @brief Is signal connected
     */
    bool isConnected() const
    {
        return !wl_list_empty(&m_wrapper.listener.link);
    }

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

private:
    static void onSignal(wl_listener* listener, void* data)
    {
        reinterpret_cast<Wrapper*>(listener)->callback(data);
    }

private:
    using Callback = std::function<void(void*)>;

    struct Wrapper
    {
        wl_listener listener;
        Callback    callback;
    };
    
private:
    Wrapper m_wrapper;
};

}

#endif //SYCAMORE_LISTENER_H