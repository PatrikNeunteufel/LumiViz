/**
 ****************************************************************************************
 * @file   BaseManager.hpp
 * @brief  Lifecycle base class for long-lived, non-UI system services.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <string>
#include <string_view>

#include "common/Attributes.hpp"

namespace viz::core{
    class ServiceContainer;
    class EventBus;
    class CommandBus;

    enum class ManagerState : unsigned char
    {
        Constructed = 0,
        Initialized,
        Started,
        Stopped,
        Shutdown
    };

    class BaseManager
    {
      public:
        BaseManager(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept;
        virtual ~BaseManager();

        BaseManager(const BaseManager&) = delete;
        BaseManager& operator=(const BaseManager&) = delete;
        BaseManager(BaseManager&&) = delete;
        BaseManager& operator=(BaseManager&&) = delete;

        VIZ_NODISCARD bool initialize() noexcept;
        VIZ_NODISCARD bool start() noexcept;
        VIZ_NODISCARD bool tick(float dt) noexcept;
        VIZ_NODISCARD bool stop() noexcept;
        VIZ_NODISCARD bool shutdown() noexcept;

        VIZ_NODISCARD ManagerState state() const noexcept;
        VIZ_NODISCARD bool isInitialized() const noexcept;
        VIZ_NODISCARD bool isRunning() const noexcept;
        VIZ_NODISCARD bool isShutdown() const noexcept;
        VIZ_NODISCARD std::string const& lastError() const noexcept;

      protected:
        virtual bool onInitialize() noexcept;
        virtual bool onStart() noexcept;
        virtual bool onTick(float /*dt*/) noexcept;
        virtual void onStop() noexcept;
        virtual void onShutdown() noexcept;

        VIZ_NODISCARD ServiceContainer& services() noexcept;
        VIZ_NODISCARD EventBus& events() noexcept;
        VIZ_NODISCARD CommandBus& commands() noexcept;

        VIZ_NODISCARD ServiceContainer const& services() const noexcept;
        VIZ_NODISCARD EventBus const& events() const noexcept;
        VIZ_NODISCARD CommandBus const& commands() const noexcept;

        void setError(std::string_view msg) noexcept;
        void clearError() noexcept;

      private:
        ServiceContainer& m_services;
        EventBus& m_events;
        CommandBus& m_commands;

        ManagerState m_state{ManagerState::Constructed};
        std::string m_lastError;
    };


} // namespace viz::core
