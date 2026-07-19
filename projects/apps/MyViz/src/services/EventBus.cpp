/**
 ****************************************************************************************
 * @file   EventBus.cpp
 * @brief  EventBus implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "services/EventBus.hpp"

// =============================================================================
// IEventBus Implementation
// =============================================================================

IEventBus::SubscriberId EventBus::doSubscribe(
    const std::type_info& type,
    std::function<void(const Event&)> handler,
    int priority,
    std::weak_ptr<void> owner,
    bool hasOwner)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto key = std::type_index(type);
    auto id = m_nextId++;

    auto& subscribers = m_subscribers[key];
    subscribers.push_back(
        Subscriber{id, priority, std::move(handler), std::move(owner), hasOwner});

    // Sort by priority (lower first)
    std::sort(subscribers.begin(), subscribers.end());

    return id;
}

void EventBus::unsubscribe(SubscriberId id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& [type, subscribers] : m_subscribers)
    {
        auto it = std::remove_if(subscribers.begin(), subscribers.end(),
            [id](const Subscriber& s) { return s.id == id; });

        if (it != subscribers.end())
        {
            subscribers.erase(it, subscribers.end());
            return;
        }
    }
}

void EventBus::doPublish(const std::type_info& type, const Event& event)
{
    std::vector<Subscriber> subscribersCopy;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(type);
        auto it = m_subscribers.find(key);
        if (it == m_subscribers.end())
        {
            return;
        }

        // Purge expired weak subscriptions (owner died)
        auto& subscribers = it->second;
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                [](const Subscriber& s) { return s.expired(); }),
            subscribers.end());

        // Copy subscribers to avoid holding lock during callbacks
        subscribersCopy = subscribers;
    }

    // Dispatch to all subscribers
    for (const auto& subscriber : subscribersCopy)
    {
        if (event.isConsumed())
        {
            break;
        }

        subscriber.handler(event);
    }
}

void EventBus::doQueue(const std::type_info& type, std::unique_ptr<Event> event)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);

    m_eventQueue.push_back(QueuedEvent{
        std::type_index(type),
        std::move(event)
    });
}

void EventBus::dispatchQueued()
{
    std::vector<QueuedEvent> eventsToDispatch;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        eventsToDispatch = std::move(m_eventQueue);
        m_eventQueue.clear();
    }

    for (auto& queuedEvent : eventsToDispatch)
    {
        std::vector<Subscriber> subscribersCopy;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_subscribers.find(queuedEvent.type);
            if (it == m_subscribers.end())
            {
                continue;
            }

            // Purge expired weak subscriptions (owner died)
            auto& subscribers = it->second;
            subscribers.erase(
                std::remove_if(subscribers.begin(), subscribers.end(),
                    [](const Subscriber& s) { return s.expired(); }),
                subscribers.end());

            subscribersCopy = subscribers;
        }

        for (const auto& subscriber : subscribersCopy)
        {
            if (queuedEvent.event->isConsumed())
            {
                break;
            }

            subscriber.handler(*queuedEvent.event);
        }
    }
}

std::size_t EventBus::doSubscriberCount(const std::type_info& type) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto key = std::type_index(type);
    auto it = m_subscribers.find(key);
    if (it == m_subscribers.end())
    {
        return 0;
    }

    return it->second.size();
}

void EventBus::clear()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_subscribers.clear();
    }

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_eventQueue.clear();
    }
}
