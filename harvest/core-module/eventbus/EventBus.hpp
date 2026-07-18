/**
 ****************************************************************************************
 * @file   EventBus.hpp
 * @brief  Type-safe, synchronous EventBus for viz2025.
 *
 * - Topic == event type (T)
 * - StrongId-based HandlerId + RAII SubscriberHandle
 * - Weak-owner subscriptions (std::weak_ptr)
 * - Thread-safe (shared_mutex), snapshot dispatch
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/Attributes.hpp"
#include "common/Types.hpp"

namespace viz::core
{
    using HandlerId = viz::types::StrongId<viz::types::u64, struct EventHandlerIdTag>;

    namespace detail
    {
        // Unique per-T address used as robust map key (ODR-safe, no RTTI dependencies).
        template<typename T> inline const void* topic_key()
        {
            static int unique; // one per T, unique address
            return &unique;
        }
    } // namespace detail

    class EventBus
    {
      public:
        class SubscriberHandle
        {
          public:
            SubscriberHandle() = default;
            explicit SubscriberHandle(std::function<void()> unsubscriber) noexcept;
            SubscriberHandle(SubscriberHandle&& other) noexcept;
            SubscriberHandle& operator=(SubscriberHandle&& other) noexcept;
            SubscriberHandle(const SubscriberHandle&) = delete;
            SubscriberHandle& operator=(const SubscriberHandle&) = delete;
            ~SubscriberHandle();
            void reset() noexcept;
            VIZ_NODISCARD explicit operator bool() const noexcept;

          private:
            std::function<void()> m_unsubscriber{};
        };

      private:
         struct Slot
    {
        HandlerId                        id{};
        std::function<void(const void*)> fn;
        std::weak_ptr<void>              weak_owner;
        bool                             has_owner{false}; ///< true if weak_owner is used
    };
    using Slots = std::vector<Slot>;

      public:
        EventBus() = default;
        ~EventBus() = default;
        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        template<typename T, typename F> VIZ_NODISCARD HandlerId subscribe(F&& cb);

        template<typename T, typename Owner, typename F>
        VIZ_NODISCARD HandlerId subscribeWeak(const std::weak_ptr<Owner>& owner, F&& cb);

        // Convenience: shared_ptr Overloads
        template<typename T, typename Owner, typename F>
        VIZ_NODISCARD HandlerId subscribeWeak(const std::shared_ptr<Owner>& owner, F&& cb);

        template<typename T, typename F> VIZ_NODISCARD SubscriberHandle subscribeScoped(F&& cb);

        template<typename T, typename Owner, typename F>
        VIZ_NODISCARD SubscriberHandle subscribeScopedWeak(const std::weak_ptr<Owner>& owner, F&& cb);

        // Convenience: shared_ptr Overload
        template<typename T, typename Owner, typename F>
        VIZ_NODISCARD SubscriberHandle subscribeScopedWeak(const std::shared_ptr<Owner>& owner, F&& cb);

        template<typename T> void publish(const T& evt);

        template<typename T> bool unsubscribe(HandlerId id);

        template<typename T> VIZ_NODISCARD viz::types::u64 subscriberCount() const;

        template<typename T> void clearTopic();

        void clearAll();

      private:
        template<typename T, typename F>
        HandlerId subscribeImpl(std::weak_ptr<void> erasedOwner, bool hasOwner, F&& cb);

        template<typename T> SubscriberHandle makeHandle(HandlerId id);

      private:
        using TopicKey = const void*; // 👈 robust key
        mutable std::shared_mutex m_mutex;
        std::unordered_map<TopicKey, Slots> m_map;
        std::atomic<viz::types::u64> m_nextId{0};
    };
} // namespace viz::core

#include "EventBus.tpp"
