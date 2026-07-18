/**
 ****************************************************************************************
 * @file   BaseManager.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "BaseManager.hpp"

#include "core/servicecontainer/ServiceContainer.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/eventbus/EventBus.hpp"

namespace viz::core
{
    BaseManager::BaseManager(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept
        : m_services(services), m_events(events), m_commands(commands){
    }
    BaseManager::~BaseManager() {}

    VIZ_NODISCARD bool BaseManager::initialize() noexcept 
    {
        if (VIZ_UNLIKELY(m_state != ManagerState::Constructed))
        {
            setError("initialize() called in invalid state");
            return false;
        }
        if (!onInitialize())
        {
            setError("onInitialize() failed");
            return false;
        }
        m_state = ManagerState::Initialized;
        clearError();
        return true;
    }
    VIZ_NODISCARD bool BaseManager::start() noexcept 
    {
        if (VIZ_UNLIKELY(m_state != ManagerState::Initialized))
        {
            setError("start() requires Initialized state");
            return false;
        }
        if (!onStart())
        {
            setError("onStart() failed");
            return false;
        }
        m_state = ManagerState::Started;
        clearError();
        return true;
    }
    VIZ_NODISCARD bool BaseManager::tick(float dt) noexcept
    {
        if (VIZ_UNLIKELY(m_state != ManagerState::Started))
        {
            setError("tick() requires Started state");
            return false;
        }
        if (!onTick(dt))
        {
            setError("onTick() reported failure");
            return false;
        }
        return true;
    }
    VIZ_NODISCARD bool BaseManager::stop() noexcept
    {
        if (m_state == ManagerState::Started)
        {
            onStop();
            m_state = ManagerState::Stopped;
            clearError();
            return true;
        }
        setError("stop() requires Started state");
        return false;
    }
    VIZ_NODISCARD bool BaseManager::shutdown() noexcept
    {
        if (m_state == ManagerState::Initialized || m_state == ManagerState::Stopped)
        {
            onShutdown();
            m_state = ManagerState::Shutdown;
            clearError();
            return true;
        }
        setError("shutdown() requires Initialized or Stopped state");
        return false;
    }

    VIZ_NODISCARD ManagerState BaseManager::state() const noexcept
    {
        return m_state;
    }
    VIZ_NODISCARD bool BaseManager::isInitialized() const noexcept
    {
        return m_state == ManagerState::Initialized;
    }
    VIZ_NODISCARD bool BaseManager::isRunning() const noexcept
    {
        return m_state == ManagerState::Started;
    }
    VIZ_NODISCARD bool BaseManager::isShutdown() const noexcept
    {
        return m_state == ManagerState::Shutdown;
    }
    VIZ_NODISCARD std::string const& BaseManager::lastError() const noexcept
    {
        return m_lastError;
    }

    bool BaseManager::onInitialize() noexcept
    {
        return true;
    }
    bool BaseManager::onStart() noexcept
    {
        return true;
    }
    bool BaseManager::onTick(float /*dt*/) noexcept
    {
        return true;
    }
    void BaseManager::onStop() noexcept {}
    void BaseManager::onShutdown() noexcept {}

    VIZ_NODISCARD ServiceContainer& BaseManager::services() noexcept
    {
        return m_services;
    }
    VIZ_NODISCARD EventBus& BaseManager::events() noexcept
    {
        return m_events;
    }
    VIZ_NODISCARD CommandBus& BaseManager::commands() noexcept
    {
        return m_commands;
    }

    VIZ_NODISCARD ServiceContainer const& BaseManager::services() const noexcept
    {
        return m_services;
    }
    VIZ_NODISCARD EventBus const& BaseManager::events() const noexcept
    {
        return m_events;
    }
    VIZ_NODISCARD CommandBus const& BaseManager::commands() const noexcept
    {
        return m_commands;
    }

    void BaseManager::setError(std::string_view msg) noexcept
    {
        m_lastError.assign(msg);
    }
    void BaseManager::clearError() noexcept
    {
        m_lastError.clear();
    }
}



