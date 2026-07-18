/**
 ****************************************************************************************
 * @file   ServiceScope.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */

#include <mutex>

#include "ServiceContainer.hpp"
#include "detail/ResolutionStack.hpp"
#include "ServiceScope.hpp"

namespace viz::core
{

    // Small RAII guard using internal TLS stack for cycle detection.
    namespace
    {
        struct StackGuard
        {
            explicit StackGuard(const std::type_index& idx) : m_idx(idx)
            {
                auto& st = detail::resolvingStack();
                if (detail::contains(st, m_idx))
                {
                    throw ServiceCycleError("Cyclic dependency detected");
                }
                st.push_back(m_idx);
                m_active = true;
            }
            ~StackGuard() noexcept
            {
                if (!m_active)
                    return;
                auto& st = detail::resolvingStack();
                if (!st.empty() && st.back() == m_idx)
                {
                    st.pop_back();
                }
            }

          private:
            std::type_index m_idx;
            bool m_active{false};
        };
    } // anonymous



    ServiceScope::ServiceScope(ServiceContainer& root) : m_root(&root) {}

    ServiceScope::ServiceScope(ServiceScope&& other) noexcept
        : m_root(other.m_root), m_scopedCache(std::move(other.m_scopedCache))
    {
        other.m_root = nullptr;
    }

    ServiceScope& ServiceScope::operator=(ServiceScope&& other) noexcept
    {
        if (this != &other)
        {
            m_root = other.m_root;
            m_scopedCache = std::move(other.m_scopedCache);
            other.m_root = nullptr;
        }
        return *this;
    }

    ServiceScope::~ServiceScope() noexcept = default;

    IServiceResolver::AnyPtr ServiceScope::resolveImpl(const std::type_info& ti)
    {
        if (!m_root)
            throw ServiceError("Scope not bound to root container");

        const std::type_index idx{ti};

        // 1) Zuerst Lifetime & Factory beschaffen (unter Root-Lock), ABER noch kein Guard!
        std::function<std::shared_ptr<void>(IServiceResolver&)> factory;
        ServiceLifetime lifetime{};

        {
            std::lock_guard<std::mutex> lock(m_root->m_mutex);
            auto it = m_root->m_entries.find(idx);
            if (it == m_root->m_entries.end())
            {
                throw ServiceError(std::string("Service not registered: ") + ServiceContainer::typeName(ti));
            }

            auto& e = it->second;
            lifetime = e.lifetime;

            if (lifetime == ServiceLifetime::Singleton)
            {
                // Für Singletons sofort an Root delegieren – KEIN Guard im Scope.
                // (Root übernimmt lazy construction & caching.)
                // WICHTIG: außerhalb des Locks rufen:
            }
            else if (lifetime == ServiceLifetime::Scoped)
            {
                if (auto scIt = m_scopedCache.find(idx); scIt != m_scopedCache.end())
                {
                    return {scIt->second};
                }
                if (!e.factory)
                {
                    throw ServiceError(std::string("Scoped service missing factory: ") +
                                       ServiceContainer::typeName(ti));
                }
                factory = e.factory; // call outside lock
            }
            else
            { // Transient
                if (!e.factory)
                {
                    throw ServiceError(std::string("Transient service missing factory: ") +
                                       ServiceContainer::typeName(ti));
                }
                factory = e.factory; // call outside lock
            }
        }

        // 2) Singleton jetzt ohne Guard delegieren
        if (lifetime == ServiceLifetime::Singleton)
        {
            return m_root->resolveImpl(ti);
        }

        // 3) Für Scoped/Transient: JETZT erst Guard anlegen (Zyklus-Erkennung)
        struct Guard
        {
            explicit Guard(const std::type_index& idx) : m_idx(idx)
            {
                auto& st = detail::resolvingStack();
                if (detail::contains(st, m_idx))
                {
                    throw ServiceCycleError("Cyclic dependency detected");
                }
                st.push_back(m_idx);
                m_active = true;
            }
            ~Guard() noexcept
            {
                if (!m_active)
                    return;
                auto& st = detail::resolvingStack();
                if (!st.empty() && st.back() == m_idx)
                    st.pop_back();
            }
            std::type_index m_idx;
            bool m_active{false};
        } guard{idx};

        // 4) Erzeugen (außerhalb jeglichen Locks)
        auto created = factory(*this);
        if (!created)
        {
            throw ServiceError(std::string("Factory returned null for type: ") + ServiceContainer::typeName(ti));
        }

        if (lifetime == ServiceLifetime::Scoped)
        {
            m_scopedCache.emplace(idx, created);
        }

        return {created};
    }


} // namespace viz::core
