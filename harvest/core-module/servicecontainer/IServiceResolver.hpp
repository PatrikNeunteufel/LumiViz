/**
 ****************************************************************************************
 * @file   IServiceResolver.hpp
 * @brief  Abstract resolver interface used by factories to resolve dependencies.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <memory>
#include <typeinfo>

#include "common/Attributes.hpp" // VIZ_NODISCARD, VIZ_LIKELY, VIZ_UNLIKELY

namespace viz::core
{
    /**
     * @brief Abstract base to resolve services by type.
     *        Implemented by ServiceContainer (root) and ServiceScope (per-scope).
     */
    class IServiceResolver
    {
      protected:
        struct AnyPtr
        {
            std::shared_ptr<void> p;
            template<typename T> std::shared_ptr<T> cast()
            {
                return std::static_pointer_cast<T>(p);
            }
        };

        /// Implemented by derived types to resolve a type-erased pointer.
        virtual AnyPtr resolveImpl(const std::type_info& ti) = 0;

      public:
        virtual ~IServiceResolver() noexcept = default;

        /// Resolve a registered service; throws on failure.
        template<typename T> VIZ_NODISCARD std::shared_ptr<T> get();

        /// Try to resolve a service; returns nullptr on failure.
        template<typename T> VIZ_NODISCARD std::shared_ptr<T> tryGet() noexcept;
    };

} // namespace viz::core

#include "IServiceResolver.tpp"
