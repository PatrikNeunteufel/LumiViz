/**
 ****************************************************************************************
 * @file   ServiceContainer.tpp
 * @brief  Template definitions for ServiceContainer.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
/**
 * @file    ServiceContainer.tpp
 * @brief   
 */

#include "ServiceContainer.hpp"

namespace viz::core
{

    template<typename T>
    void ServiceContainer::addSingleton(std::shared_ptr<T> instance, std::string debugName, bool eager)
    {
        if (!instance)
            throw ServiceError("addSingleton: instance == null");
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& e = m_entries[std::type_index(typeid(T))];
        e.lifetime = ServiceLifetime::Singleton;
        e.factory = nullptr;
        e.singletonInstance = std::move(instance);
        e.debugName = debugName.empty() ? typeName(typeid(T)) : std::move(debugName);
        e.eager = eager;
    }

    template<typename T>
    void ServiceContainer::addSingletonFactory(std::function<std::shared_ptr<T>(IServiceResolver&)> f,
                                               std::string debugName, bool eager)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& e = m_entries[std::type_index(typeid(T))];
        e.lifetime = ServiceLifetime::Singleton;
        e.factory = [f = std::move(f)](IServiceResolver& r) { return f(r); };
        e.singletonInstance.reset();
        e.debugName = debugName.empty() ? typeName(typeid(T)) : std::move(debugName);
        e.eager = eager;
    }

    template<typename T>
    void ServiceContainer::addScoped(std::function<std::shared_ptr<T>(IServiceResolver&)> f, std::string debugName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& e = m_entries[std::type_index(typeid(T))];
        e.lifetime = ServiceLifetime::Scoped;
        e.factory = [f = std::move(f)](IServiceResolver& r) { return f(r); };
        e.singletonInstance.reset();
        e.debugName = debugName.empty() ? typeName(typeid(T)) : std::move(debugName);
        e.eager = false;
    }

    template<typename T>
    void ServiceContainer::addTransient(std::function<std::shared_ptr<T>(IServiceResolver&)> f, std::string debugName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& e = m_entries[std::type_index(typeid(T))];
        e.lifetime = ServiceLifetime::Transient;
        e.factory = [f = std::move(f)](IServiceResolver& r) { return f(r); };
        e.singletonInstance.reset();
        e.debugName = debugName.empty() ? typeName(typeid(T)) : std::move(debugName);
        e.eager = false;
    }

    template<typename T> void ServiceContainer::replaceWithSingleton(std::shared_ptr<T> instance, std::string debugName)
    {
        addSingleton<T>(std::move(instance), std::move(debugName), /*eager*/ false);
    }

    template<typename T> bool ServiceContainer::remove()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries.erase(std::type_index(typeid(T))) > 0;
    }

    template<typename T> bool ServiceContainer::isRegistered() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries.find(std::type_index(typeid(T))) != m_entries.end();
    }

    template<typename T> std::shared_ptr<T> ServiceContainer::get()
    {
        return resolveImpl(typeid(T)).template cast<T>();
    }

    template<typename T> std::shared_ptr<T> ServiceContainer::tryGet() noexcept
    {
        try
        {
            return get<T>();
        }
        catch (...)
        {
            return {};
        }
    }

} // namespace viz::core
