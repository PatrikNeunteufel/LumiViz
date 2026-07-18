/**
 ****************************************************************************************
 * @file   ServiceContainer.hpp
 * @brief  Root DI container with RAII scopes and lifetimes
 *          (Singleton/Scoped/Transient). Thread-safe resolution, factory-based
 *          construction, cycle detection (via internal TLS stack), and optional
 *          eager initialization.
 *
 * @note    Templated member functions are defined in ServiceContainer.tpp.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "IServiceResolver.hpp"
#include "ServiceErrors.hpp"
#include "ServiceLifetime.hpp"
#include "common/Attributes.hpp" // VIZ_NODISCARD, VIZ_LIKELY, VIZ_UNLIKELY

namespace viz::core
{

    class ServiceScope;

    /**
     * @brief Root container holding registrations and the singleton cache.
     */
    class ServiceContainer : public IServiceResolver
    {
      public:
        ServiceContainer();
        ServiceContainer(const ServiceContainer&) = delete;
        ServiceContainer& operator=(const ServiceContainer&) = delete;

        // ----- Registration API (templates in .tpp) ------------------------------

        template<typename T>
        void addSingleton(std::shared_ptr<T> instance, std::string debugName = {}, bool eager = false);

        template<typename T>
        void addSingletonFactory(std::function<std::shared_ptr<T>(IServiceResolver&)> f, std::string debugName = {},
                                 bool eager = false);

        template<typename T>
        void addScoped(std::function<std::shared_ptr<T>(IServiceResolver&)> f, std::string debugName = {});

        template<typename T>
        void addTransient(std::function<std::shared_ptr<T>(IServiceResolver&)> f, std::string debugName = {});

        template<typename T> void replaceWithSingleton(std::shared_ptr<T> instance, std::string debugName = {});

        template<typename T> bool remove();

        template<typename T> VIZ_NODISCARD bool isRegistered() const;

        template<typename T> VIZ_NODISCARD std::shared_ptr<T> get();

        template<typename T> VIZ_NODISCARD std::shared_ptr<T> tryGet() noexcept;

        // ----- Scopes ------------------------------------------------------------

        /// Create a new scope (RAII lifetime for scoped instances).
        VIZ_NODISCARD ServiceScope createScope();

        // ----- Lifecycle ---------------------------------------------------------

        /// Build all eager-marked singletons upfront (e.g., at app startup).
        void buildSingletons();

        /// Clear all registrations and caches.
        void clear();

      protected:
        // IServiceResolver
        AnyPtr resolveImpl(const std::type_info& ti) override;

      private:
        friend class ServiceScope;

struct Entry
        {
            ServiceLifetime lifetime{};
            std::function<std::shared_ptr<void>(IServiceResolver&)> factory{};
            std::shared_ptr<void> singletonInstance{};
            std::string debugName{};
            bool eager{false};
            std::once_flag initFlag; // <-- NEU: garantiert genau eine Konstruktion
        };

        // Diagnostics helper (portable name)
        static std::string typeName(const std::type_info& ti);

        // Private helper to resolve by std::type_index (used by buildSingletons)
        AnyPtr resolveByIndex(const std::type_index& idx);

        // Mutex protects m_entries and singleton cache writes
        mutable std::mutex m_mutex;
        std::unordered_map<std::type_index, Entry> m_entries;
    };

} // namespace viz::core

#include "ServiceContainer.tpp"
