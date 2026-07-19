/**
 ****************************************************************************************
 * @file   IEventBus.hpp
 * @brief  Interface for the EventBus publish/subscribe system
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Event Bus
 *
 * Der EventBus implementiert das **Publish/Subscribe Pattern** für lose Kopplung:
 *
 * ```
 * ┌─────────────┐     publish()     ┌─────────────┐     notify()     ┌─────────────┐
 * │  Publisher  │ ─────────────────► │  EventBus   │ ─────────────────► │ Subscriber  │
 * │ (AudioEngine)│                   │             │                   │  (Panel)    │
 * └─────────────┘                   └─────────────┘                   └─────────────┘
 *                                          │
 *                                          ├──────────────────────────► Subscriber 2
 *                                          │
 *                                          └──────────────────────────► Subscriber 3
 * ```
 *
 * ### Verwendung
 *
 * ```cpp
 * // 1. Event definieren
 * struct AudioDataEvent : public Event {
 *     EVENT_TYPE_NAME("AudioDataEvent")
 *     const float* spectrum;
 *     int size;
 * };
 *
 * // 2. Subscriben
 * auto id = eventBus.subscribe<AudioDataEvent>([](const AudioDataEvent& e) {
 *     updateVisualization(e.spectrum, e.size);
 * });
 *
 * // 3. Publishen
 * eventBus.publish(AudioDataEvent{spectrum, 1024});
 *
 * // 4. Unsubscribe (z.B. im Destruktor)
 * eventBus.unsubscribe(id);
 * ```
 ****************************************************************************************
 */

#pragma once

#include "events/Event.hpp"

#include <functional>
#include <cstdint>
#include <memory>
#include <typeinfo>
#include <type_traits>

/**
 * @class IEventBus
 * @brief Interface for the EventBus
 *
 * Ermöglicht Publish/Subscribe für lose Kopplung zwischen Komponenten.
 */
class IEventBus
{
public:
    /// Subscriber-Handle für Unsubscription
    using SubscriberId = std::uint64_t;

    /**
     * @class SubscriberHandle
     * @brief RAII handle — unsubscribes automatically on destruction
     *
     * Move-only. Obtain via subscribeScoped()/subscribeScopedWeak(). The handle
     * must not outlive the EventBus it was created from (in MyViz the bus lives
     * in the ServiceContainer and outlives all panels/widgets).
     */
    class SubscriberHandle
    {
    public:
        SubscriberHandle() = default;
        explicit SubscriberHandle(std::function<void()> unsubscriber) noexcept
            : m_unsubscriber(std::move(unsubscriber)) {}

        SubscriberHandle(SubscriberHandle&& other) noexcept
            : m_unsubscriber(std::move(other.m_unsubscriber))
        {
            other.m_unsubscriber = nullptr;
        }

        SubscriberHandle& operator=(SubscriberHandle&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_unsubscriber = std::move(other.m_unsubscriber);
                other.m_unsubscriber = nullptr;
            }
            return *this;
        }

        SubscriberHandle(const SubscriberHandle&) = delete;
        SubscriberHandle& operator=(const SubscriberHandle&) = delete;

        ~SubscriberHandle() { reset(); }

        /// @brief Unsubscribe now (idempotent)
        void reset() noexcept
        {
            if (m_unsubscriber)
            {
                m_unsubscriber();
                m_unsubscriber = nullptr;
            }
        }

        /// @brief True while the subscription is active
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_unsubscriber);
        }

    private:
        std::function<void()> m_unsubscriber{};
    };

    virtual ~IEventBus() = default;

    // =========================================================================
    // Subscription
    // =========================================================================

    /**
     * @brief Subscribe to events of type T
     *
     * @tparam T Event type (must derive from Event)
     * @param handler Callback function
     * @return Subscriber ID for later unsubscription
     *
     * @code
     * auto id = eventBus.subscribe<AudioDataEvent>([](const AudioDataEvent& e) {
     *     processAudio(e.spectrum, e.size);
     * });
     * @endcode
     */
    template<typename T>
    SubscriberId subscribe(std::function<void(const T&)> handler)
    {
        return subscribe<T>(std::move(handler), 0);
    }

    /**
     * @brief Subscribe to events with priority
     *
     * @tparam T Event type (must derive from Event)
     * @param handler Callback function
     * @param priority Lower values are called first (default: 0)
     * @return Subscriber ID for later unsubscription
     */
    template<typename T>
    SubscriberId subscribe(std::function<void(const T&)> handler, int priority)
    {
        static_assert(std::is_base_of_v<Event, T>,
                      "T must derive from Event");

        auto wrapper = [handler = std::move(handler)](const Event& e) {
            handler(static_cast<const T&>(e));
        };

        return doSubscribe(typeid(T), std::move(wrapper), priority,
                           std::weak_ptr<void>{}, false);
    }

    /**
     * @brief Subscribe bound to an owner's lifetime (weak subscription)
     *
     * The handler is only invoked while the owner is alive; expired
     * subscriptions are purged automatically on the next publish.
     *
     * @tparam T Event type (must derive from Event)
     * @tparam Owner Owner object type (held via shared_ptr/weak_ptr)
     * @param owner Owner whose lifetime gates the subscription
     * @param handler Callback function
     * @param priority Lower values are called first (default: 0)
     * @return Subscriber ID (manual unsubscribe optional)
     */
    template<typename T, typename Owner>
    SubscriberId subscribeWeak(const std::weak_ptr<Owner>& owner,
                               std::function<void(const T&)> handler,
                               int priority = 0)
    {
        static_assert(std::is_base_of_v<Event, T>,
                      "T must derive from Event");

        auto wrapper = [owner, handler = std::move(handler)](const Event& e) {
            if (auto locked = owner.lock())
            {
                handler(static_cast<const T&>(e));
            }
        };

        return doSubscribe(typeid(T), std::move(wrapper), priority,
                           std::weak_ptr<void>(owner), true);
    }

    /// @brief Convenience overload taking a shared_ptr owner
    template<typename T, typename Owner>
    SubscriberId subscribeWeak(const std::shared_ptr<Owner>& owner,
                               std::function<void(const T&)> handler,
                               int priority = 0)
    {
        return subscribeWeak<T, Owner>(std::weak_ptr<Owner>(owner),
                                       std::move(handler), priority);
    }

    /**
     * @brief Subscribe with RAII lifetime (auto-unsubscribe on handle destruction)
     *
     * @code
     * m_eventSubscriptions.push_back(
     *     bus->subscribeScoped<VisualizerChangedEvent>([this](const auto& e) { ... }));
     * @endcode
     */
    template<typename T>
    [[nodiscard]] SubscriberHandle subscribeScoped(
        std::function<void(const T&)> handler, int priority = 0)
    {
        auto id = subscribe<T>(std::move(handler), priority);
        return makeHandle(id);
    }

    /// @brief RAII handle + owner-gated handler in one (see subscribeWeak)
    template<typename T, typename Owner>
    [[nodiscard]] SubscriberHandle subscribeScopedWeak(
        const std::shared_ptr<Owner>& owner,
        std::function<void(const T&)> handler, int priority = 0)
    {
        auto id = subscribeWeak<T, Owner>(owner, std::move(handler), priority);
        return makeHandle(id);
    }

    /**
     * @brief Unsubscribe by handle
     * @param id Subscriber ID returned from subscribe()
     */
    virtual void unsubscribe(SubscriberId id) = 0;

    // =========================================================================
    // Publishing
    // =========================================================================

    /**
     * @brief Publish event to all subscribers (synchronous)
     *
     * @tparam T Event type
     * @param event Event to publish
     *
     * @code
     * eventBus.publish(AudioDataEvent{spectrum, 1024});
     * @endcode
     */
    template<typename T>
    void publish(const T& event)
    {
        static_assert(std::is_base_of_v<Event, T>,
                      "T must derive from Event");

        doPublish(typeid(T), event);
    }

    /**
     * @brief Queue event for later dispatch (thread-safe)
     *
     * Useful for cross-thread communication.
     *
     * @tparam T Event type
     * @param event Event to queue
     */
    template<typename T>
    void queue(const T& event)
    {
        static_assert(std::is_base_of_v<Event, T>,
                      "T must derive from Event");

        doQueue(typeid(T), std::make_unique<T>(event));
    }

    /**
     * @brief Dispatch all queued events
     *
     * Should be called from the main thread (e.g., in the render loop).
     */
    virtual void dispatchQueued() = 0;

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Get subscriber count for event type
     * @tparam T Event type
     * @return Number of subscribers
     */
    template<typename T>
    [[nodiscard]] std::size_t subscriberCount() const
    {
        static_assert(std::is_base_of_v<Event, T>,
                      "T must derive from Event");

        return doSubscriberCount(typeid(T));
    }

    /**
     * @brief Clear all subscriptions
     */
    virtual void clear() = 0;

protected:
    /**
     * @brief Build a teardown-safe RAII handle for the given subscription
     *
     * The handle checks the bus liveness token before calling back — if the
     * bus was already destroyed (teardown-order!), reset() is a no-op instead
     * of an access violation.
     */
    [[nodiscard]] SubscriberHandle makeHandle(SubscriberId id)
    {
        return SubscriberHandle(
            [this, id, token = std::weak_ptr<void>(m_liveToken)]() {
                if (auto alive = token.lock())
                {
                    unsubscribe(id);
                }
            });
    }

    // Internal virtual methods for template dispatch
    virtual SubscriberId doSubscribe(
        const std::type_info& type,
        std::function<void(const Event&)> handler,
        int priority,
        std::weak_ptr<void> owner,
        bool hasOwner) = 0;

    virtual void doPublish(const std::type_info& type, const Event& event) = 0;

    virtual void doQueue(const std::type_info& type,
                         std::unique_ptr<Event> event) = 0;

    virtual std::size_t doSubscriberCount(const std::type_info& type) const = 0;

private:
    /// Liveness token — expires with the bus; guards RAII handles (makeHandle).
    std::shared_ptr<void> m_liveToken = std::make_shared<char>('\0');
};
