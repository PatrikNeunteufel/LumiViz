/**
 ****************************************************************************************
 * @file   ServiceContainer.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include <utility>

#include "ServiceContainer.hpp"
#include "ServiceScope.hpp"
#include "detail/ResolutionStack.hpp"

namespace viz::core
{
    // ----- Portable type name ----------------------------------------------------
    std::string ServiceContainer::typeName(const std::type_info& ti)
    {
        // Keep ABI-independent; good enough for diagnostics.
        return ti.name();
    }

    // ----- RAII guard for cycle detection ---------------------------------------
    namespace
    {
        struct StackGuard
        {
            explicit StackGuard(const std::type_index& idx) : m_idx(idx)
            {
                auto& st = detail::resolvingStack();
                if (detail::contains(st, m_idx))
                {
                    // Throw BEFORE modifying the stack; m_active remains false.
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
                    st.pop_back(); // pop only our own frame
                }
                // Defensive: if not top, leave the stack untouched.
            }

          private:
            std::type_index m_idx;
            bool m_active{false};
        };
    } // anonymous


    // ----- Ctor ------------------------------------------------------------------
    ServiceContainer::ServiceContainer() = default;

    // ----- IServiceResolver impl (root resolution) -------------------------------
    IServiceResolver::AnyPtr ServiceContainer::resolveByIndex(const std::type_index& idx)
    {
        StackGuard guard{idx};

        std::function<std::shared_ptr<void>(IServiceResolver&)> factory;
        Entry* entryPtr = nullptr;
        ServiceLifetime lifetime{};

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_entries.find(idx);
            if (it == m_entries.end())
            {
                throw ServiceError(std::string("Service not registered: ") + idx.name());
            }

            auto& e = it->second;
            lifetime = e.lifetime;

            if (lifetime == ServiceLifetime::Singleton)
            {
                if (e.singletonInstance)
                {
                    return {e.singletonInstance}; // bereits initialisiert
                }
                if (!e.factory)
                {
                    throw ServiceError(std::string("Singleton has no instance and no factory: ") + idx.name());
                }
                // Merke Factory und Entry-Adresse, dann außerhalb des Locks konstruieren
                factory = e.factory;
                entryPtr = &e;
            }
            else
            {
                if (!e.factory)
                {
                    throw ServiceError(std::string("Service missing factory: ") + idx.name());
                }
                factory = e.factory; // call outside lock
            }
        }

        if (lifetime == ServiceLifetime::Singleton)
        {
            // Genau eine Initialisierung pro Entry (Thread-sicher, ohne Container-Lock)
            std::call_once(entryPtr->initFlag,
                           [&]()
                           {
                               auto created = factory(*this);
                               if (!created)
                               {
                                   throw ServiceError(std::string("Factory returned null for type: ") + idx.name());
                               }
                               // Cache setzen (kurz sperren)
                               std::lock_guard<std::mutex> lk(m_mutex);
                               if (!entryPtr->singletonInstance)
                               {
                                   entryPtr->singletonInstance = std::move(created);
                               }
                           });

            // Nach call_once MUSS die Instanz vorhanden sein
            std::lock_guard<std::mutex> lk(m_mutex);
            return {m_entries.at(idx).singletonInstance};
        }

        // Transient/Scoped: wie gehabt
        auto created = factory(*this);
        if (!created)
        {
            throw ServiceError(std::string("Factory returned null for type: ") + idx.name());
        }
        return {created};
    }


    // ----- Scopes & lifecycle ----------------------------------------------------
    ServiceScope ServiceContainer::createScope()
    {
        return ServiceScope(*this);
    }

    void ServiceContainer::buildSingletons()
    {
        std::vector<std::type_index> list;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& [idx, e] : m_entries)
            {
                if (e.lifetime == ServiceLifetime::Singleton && e.eager && !e.singletonInstance)
                {
                    list.emplace_back(idx);
                }
            }
        }

        for (const auto& idx : list)
        {
            (void)resolveByIndex(idx); // Construct & cache; errors propagate.
        }
    }

    void ServiceContainer::clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
    }
    IServiceResolver::AnyPtr ServiceContainer::resolveImpl(const std::type_info& ti)
    {
        return resolveByIndex(std::type_index(ti));
    }
} // namespace viz::core
