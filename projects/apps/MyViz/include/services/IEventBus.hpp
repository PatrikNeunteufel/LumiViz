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

        return doSubscribe(typeid(T), std::move(wrapper), priority);
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
    // Internal virtual methods for template dispatch
    virtual SubscriberId doSubscribe(
        const std::type_info& type,
        std::function<void(const Event&)> handler,
        int priority) = 0;

    virtual void doPublish(const std::type_info& type, const Event& event) = 0;

    virtual void doQueue(const std::type_info& type,
                         std::unique_ptr<Event> event) = 0;

    virtual std::size_t doSubscriberCount(const std::type_info& type) const = 0;
};
