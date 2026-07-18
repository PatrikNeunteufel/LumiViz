/**
 ****************************************************************************************
 * @file   BaseController.hpp
 * @brief  Base class for controllers: translate user-intentions into commands, orchestrate managers.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <future>
#include <functional>

#include "common/Attributes.hpp"
#include "core/commandbus/ICommand.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
 


namespace viz::core
{
    class ServiceContainer;
    class EventBus;
    class CommandBus;
    class ICommand;

    class BaseController
    {
      public:
        BaseController(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept;
        virtual ~BaseController();

        BaseController(const BaseController&) = delete;
        BaseController& operator=(const BaseController&) = delete;
        BaseController(BaseController&&) = delete;
        BaseController& operator=(BaseController&&) = delete;

        // Lifecycle (non-template → in .cpp implementiert)
        VIZ_NODISCARD bool initialize() noexcept;
        VIZ_NODISCARD bool start() noexcept;
        VIZ_NODISCARD bool tick(float dt) noexcept;
        void stop() noexcept;

        // Command (non-template → in .cpp implementiert)
        void dispatch(ICommand& cmd) noexcept;

        
        /** @brief Synchronous execute on the command bus. */
        CommandResult execute(ICommand& cmd, const CommandContext& ctx = {}) noexcept;

        /** @brief Asynchronous submit on the command bus. */
        std::future<CommandResult> submitAsync(ICommand& cmd, CommandContext ctx = {},
                                               std::function<void(const CommandResult&)> completion = {}) noexcept;

        // Templates (müssen im TU sichtbar sein → in .tpp implementiert)
        template<typename FnDo, typename FnUndo = std::nullptr_t>
        void dispatchAdapter(std::string_view label, FnDo&& doFn, FnUndo&& undoFn = nullptr) noexcept;

        template<typename Fn> void withTransaction(std::string_view label, Fn&& fn) noexcept;

      protected:
        // Hooks (virtuell, Default trivial)
        virtual bool onInitialize() noexcept
        {
            return true;
        }
        virtual bool onStart() noexcept
        {
            return true;
        }
        virtual bool onTick(float /*dt*/) noexcept
        {
            return true;
        }
        virtual void onStop() noexcept {}

        // DI accessors
        VIZ_NODISCARD ServiceContainer& services() noexcept
        {
            return m_services;
        }
        VIZ_NODISCARD EventBus& events() noexcept
        {
            return m_events;
        }
        VIZ_NODISCARD CommandBus& commands() noexcept
        {
            return m_commands;
        }

        VIZ_NODISCARD ServiceContainer const& services() const noexcept
        {
            return m_services;
        }
        VIZ_NODISCARD EventBus const& events() const noexcept
        {
            return m_events;
        }
        VIZ_NODISCARD CommandBus const& commands() const noexcept
        {
            return m_commands;
        }

      private:
        ServiceContainer& m_services;
        EventBus& m_events;
        CommandBus& m_commands;
    };

} // namespace viz::core

// Templates sichtbar machen:
#include "BaseController.tpp"
