/**
 ****************************************************************************************
 * @file   EventBus.hpp
 * @brief  EventBus implementation with priority support and thread-safe queuing
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "IEventBus.hpp"

#include <vector>
#include <unordered_map>
#include <mutex>
#include <typeindex>
#include <algorithm>

/**
 * @class EventBus
 * @brief Thread-safe EventBus implementation
 *
 * Features:
 * - Priority-based subscriber ordering
 * - Thread-safe event queuing
 * - Event consumption (stops propagation)
 */
class EventBus : public IEventBus
{
public:
    EventBus() = default;
    ~EventBus() override = default;

    // Non-copyable
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // =========================================================================
    // IEventBus Implementation
    // =========================================================================

    void unsubscribe(SubscriberId id) override;
    void dispatchQueued() override;
    void clear() override;

protected:
    SubscriberId doSubscribe(
        const std::type_info& type,
        std::function<void(const Event&)> handler,
        int priority) override;

    void doPublish(const std::type_info& type, const Event& event) override;

    void doQueue(const std::type_info& type,
                 std::unique_ptr<Event> event) override;

    std::size_t doSubscriberCount(const std::type_info& type) const override;

private:
    // =========================================================================
    // Internal Types
    // =========================================================================

    struct Subscriber
    {
        SubscriberId id;
        int priority;
        std::function<void(const Event&)> handler;

        bool operator<(const Subscriber& other) const
        {
            return priority < other.priority;
        }
    };

    struct QueuedEvent
    {
        std::type_index type;
        std::unique_ptr<Event> event;
    };

    // =========================================================================
    // Private Members
    // =========================================================================

    mutable std::mutex m_mutex;
    std::unordered_map<std::type_index, std::vector<Subscriber>> m_subscribers;
    SubscriberId m_nextId = 1;

    std::mutex m_queueMutex;
    std::vector<QueuedEvent> m_eventQueue;
};
