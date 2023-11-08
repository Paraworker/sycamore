#ifndef SYCAMORE_LISTENER_H
#define SYCAMORE_LISTENER_H

#include "sycamore/defines.h"

#include <cassert>
#include <concepts>
#include <memory>
#include <wayland-server-core.h>

NAMESPACE_SYCAMORE_BEGIN

template<typename T>
concept IsCallback = requires(T object, void* data) {
    {object(data)} -> std::same_as<void>;
};

// A wrapper around wl_listener
class Listener {
public:
    /**
     * @brief Constructor
     */
    Listener() = default;

    /**
     * @brief Destructor
     */
    ~Listener() = default;

    /**
     * @brief Set callback
     */
    template<typename Func>
    void set(Func&& callback) {
        m_handler.reset(new Handler{std::forward<Func>(callback)});
    }

    /**
     * @brief Set callback and connect to signal
     */
    template<typename Func>
    void set(wl_signal* signal, Func&& callback) {
        m_handler.reset(new Handler{signal, std::forward<Func>(callback)});
    }

    /**
     * @brief Connect to signal
     * @note No-op if a signal is already connected
     */
    void connect(wl_signal* signal) {
        assert(!m_handler && "connect() before set()!");
        m_handler->connect(signal);
    }

    /**
     * @brief Disconnect form signal
     * @note No-op if no signal is connected
     */
    void disconnect() {
        assert(!m_handler && "disconnect() before set()!");
        m_handler->disconnect();
    }

    /**
     * @brief Is signal connected
     */
    bool isConnected() const {
        assert(!m_handler && "isConnected() before set()!");
        return m_handler->isConnected();
    }

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

private:
    struct HandlerBase {
        virtual ~HandlerBase() = default;
        virtual void connect(wl_signal* signal) = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
    };

    template<IsCallback Func>
    class Handler final : public HandlerBase {
    public:
        explicit Handler(const Func& callback)
            : m_holder({{}, onSignal}, callback), m_connected(false) {}

        explicit Handler(Func&& callback)
            : m_holder({{}, onSignal}, std::move(callback)), m_connected(false) {}

        Handler(wl_signal* signal, const Func& callback)
            : m_holder({{}, onSignal}, callback), m_connected(true) {
            wl_signal_add(signal, &m_holder.listener);
        }

        Handler(wl_signal* signal, Func&& callback)
            : m_holder({{}, onSignal}, std::move(callback)), m_connected(true) {
            wl_signal_add(signal, &m_holder.listener);
        }

        ~Handler() override { disconnect(); }

        void connect(wl_signal* signal) override {
            if (m_connected) {
                return;
            }

            wl_signal_add(signal, &m_holder.listener);

            m_connected = true;
        }

        void disconnect() override {
            if (!m_connected) {
                return;
            }

            wl_list_remove(&m_holder.listener.link);

            m_connected = false;
        }

        bool isConnected() const override { return m_connected; }

    private:
        static void onSignal(wl_listener* listener, void* data) {
            reinterpret_cast<Handler<Func>::Holder*>(listener)->callback(data);
        }

    private:
        struct Holder {
            wl_listener listener; // Wrapped listener
            Func        callback; // Callback functor
        };

    private:
        Holder m_holder;
        bool   m_connected;
    };

private:
    std::unique_ptr<HandlerBase> m_handler;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_LISTENER_H