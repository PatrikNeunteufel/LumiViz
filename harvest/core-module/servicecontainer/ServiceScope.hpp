/**
 ****************************************************************************************
 * @file   ServiceScope.hpp
 * @brief  Per-scope resolver caching Scoped services, delegating Singletons
 *          to the root container. RAII lifetime: instances are released
 *          when the scope is destroyed.
 *
 * @note    Templated member functions are defined in ServiceScope.tpp.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "IServiceResolver.hpp"
#include "ServiceErrors.hpp"
#include "ServiceLifetime.hpp"
#include "common/Attributes.hpp"

namespace viz::core
{

    class ServiceContainer;

    /**
     * @brief Scope that caches scoped instances and resolves dependencies using
     *        either itself (for scoped) or the root container (for singletons).
     */
    class ServiceScope : public IServiceResolver
    {
      public:
        explicit ServiceScope(ServiceContainer& root);
        ServiceScope(const ServiceScope&) = delete;
        ServiceScope& operator=(const ServiceScope&) = delete;

        ServiceScope(ServiceScope&& other) noexcept;
        ServiceScope& operator=(ServiceScope&& other) noexcept;

        ~ServiceScope() noexcept;

        template<typename T> VIZ_NODISCARD std::shared_ptr<T> get();

        template<typename T> VIZ_NODISCARD std::shared_ptr<T> tryGet() noexcept;

        VIZ_NODISCARD size_t scopedCount() const noexcept;

      protected:
        AnyPtr resolveImpl(const std::type_info& ti) override;

      private:
        friend class ServiceContainer; // access container privates when needed
        ServiceContainer* m_root{};
        std::unordered_map<std::type_index, std::shared_ptr<void>> m_scopedCache;
    };

} // namespace viz::core

#include "ServiceScope.tpp"
