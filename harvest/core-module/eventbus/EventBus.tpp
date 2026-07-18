/**
 ****************************************************************************************
 * @file   EventBus.tpp
 * @brief  Template definitions for EventBus.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#ifndef VIZ2025_CORE_EVENTBUS_TPP_INCLUDED
#define VIZ2025_CORE_EVENTBUS_TPP_INCLUDED

#include <algorithm>

namespace viz::core
{
    // ===== SubscriberHandle inline =====
    inline EventBus::SubscriberHandle::SubscriberHandle(std::function<void()> unsubscriber) noexcept
        : m_unsubscriber(std::move(unsubscriber))
    {
    }

    inline EventBus::SubscriberHandle::SubscriberHandle(SubscriberHandle&& other) noexcept
        : m_unsubscriber(std::move(other.m_unsubscriber))
    {
        other.m_unsubscriber = nullptr;
    }

    inline auto EventBus::SubscriberHandle::operator=(SubscriberHandle&& other) noexcept -> SubscriberHandle&
    {
        if (this != &other)
        {
            reset();
            m_unsubscriber = std::move(other.m_unsubscriber);
            other.m_unsubscriber = nullptr;
        }
        return *this;
    }

    inline EventBus::SubscriberHandle::~SubscriberHandle()
    {
        reset();
    }

    inline void EventBus::SubscriberHandle::reset() noexcept
    {
        if (m_unsubscriber)
        {
            auto fn = std::move(m_unsubscriber);
            m_unsubscriber = nullptr;
            fn();
        }
    }

    inline EventBus::SubscriberHandle::operator bool() const noexcept
    {
        return static_cast<bool>(m_unsubscriber);
    }

    // ===== EventBus templates =====
    template<typename T, typename F> HandlerId EventBus::subscribe(F&& cb)
    {
        return subscribeImpl<T>(std::weak_ptr<void>{}, false, std::forward<F>(cb));
    }

    template<typename T, typename Owner, typename F>
    HandlerId EventBus::subscribeWeak(const std::weak_ptr<Owner>& owner, F&& cb)
    {
        std::weak_ptr<void> erased(owner);
        return subscribeImpl<T>(erased, true, std::forward<F>(cb));
    }


    // shared_ptr convenience
    template<typename T, typename Owner, typename F>
    HandlerId EventBus::subscribeWeak(const std::shared_ptr<Owner>& owner, F&& cb)
    {
        return subscribeWeak<T, Owner, F>(std::weak_ptr<Owner>(owner), std::forward<F>(cb));
    }


    template<typename T, typename F> auto EventBus::subscribeScoped(F&& cb) -> SubscriberHandle
    {
        const HandlerId id = subscribe<T>(std::forward<F>(cb));
        return makeHandle<T>(id);
    }

    template<typename T, typename Owner, typename F>
    auto EventBus::subscribeScopedWeak(const std::weak_ptr<Owner>& owner, F&& cb) -> SubscriberHandle
    {
        const HandlerId id = subscribeWeak<T>(owner, std::forward<F>(cb));
        return makeHandle<T>(id);
    }

    // shared_ptr convenience
    template<typename T, typename Owner, typename F>
    auto EventBus::subscribeScopedWeak(const std::shared_ptr<Owner>& owner, F&& cb) -> SubscriberHandle
    {
        return subscribeScopedWeak<T, Owner, F>(std::weak_ptr<Owner>(owner), std::forward<F>(cb));
    }

    template<typename T> void EventBus::publish(const T& evt)
    {
        const auto key = detail::topic_key<T>();

        Slots snapshot;
        {
            std::shared_lock lock(m_mutex);
            auto it = m_map.find(key);
            if (it == m_map.end())
                return;
            snapshot = it->second; // copy
        }

        bool need_compact = false;
        for (const auto& s : snapshot)
        {
            if (s.has_owner)
            {
                if (!s.weak_owner.expired())
                {
                    if (VIZ_LIKELY(static_cast<bool>(s.fn)))
                        s.fn(static_cast<const void*>(&evt));
                }
                else
                {
                    need_compact = true;
                }
            }
            else
            {
                // no owner bound -> always call
                if (VIZ_LIKELY(static_cast<bool>(s.fn)))
                    s.fn(static_cast<const void*>(&evt));
            }
        }

        if (need_compact)
        {
            std::unique_lock lock(m_mutex);
            auto it = m_map.find(key);
            if (it != m_map.end())
            {
                auto& vec = it->second;
                vec.erase(std::remove_if(vec.begin(), vec.end(),
                                         [](const Slot& s)
                                         {
                                             return s.has_owner && !s.weak_owner.owner_before(std::weak_ptr<void>{}) &&
                                                    s.weak_owner.expired();
                                         }),
                          vec.end());
            }
        }
    }


    template<typename T> bool EventBus::unsubscribe(HandlerId id)
    {
        const auto key = detail::topic_key<T>(); // 👈
        std::unique_lock lock(m_mutex);
        auto it = m_map.find(key);
        if (it == m_map.end())
            return false;

        auto& vec = it->second;
        const auto old = vec.size();
        vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const Slot& s) { return s.id == id; }), vec.end());
        return vec.size() != old;
    }

    template<typename T> viz::types::u64 EventBus::subscriberCount() const
    {
        const auto key = detail::topic_key<T>(); // 👈
        std::shared_lock lock(m_mutex);
        auto it = m_map.find(key);
        if (it == m_map.end())
            return 0;
        return static_cast<viz::types::u64>(it->second.size());
    }

    template<typename T> void EventBus::clearTopic()
    {
        const auto key = detail::topic_key<T>(); // 👈
        std::unique_lock lock(m_mutex);
        m_map.erase(key);
    }

    inline void EventBus::clearAll()
    {
        std::unique_lock lock(m_mutex);
        m_map.clear();
    }

    template<typename T, typename F>
    HandlerId EventBus::subscribeImpl(std::weak_ptr<void> erasedOwner, bool hasOwner, F&& cb)
    {
        const auto key = detail::topic_key<T>();

        std::function<void(const void*)> fn = [f = std::forward<F>(cb)](const void* p)
        {
            const T& ref = *static_cast<const T*>(p);
            f(ref);
        };

        std::unique_lock lock(m_mutex);
        auto& vec = m_map[key];
        const HandlerId id{++m_nextId}; // starts at 1
        vec.push_back(Slot{id, std::move(fn), std::move(erasedOwner), hasOwner});
        return id;
    }


    template<typename T> auto EventBus::makeHandle(HandlerId id) -> SubscriberHandle
    {
        return SubscriberHandle([this, id]() noexcept { (void)this->unsubscribe<T>(id); });
    }

} // namespace viz::core

#endif // VIZ2025_CORE_EVENTBUS_TPP_INCLUDED
