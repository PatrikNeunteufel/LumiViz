/**
 ****************************************************************************************
 * @file   BaseAgent.hpp
 * @brief  Low-level base interface for agents (background or cooperative tasks), UI-free.
 *
 * Agents encapsulate periodic or background work under a simple lifecycle:
 *   start() → [tick(dt) ...] → stop() → join()
 *
 * Design:
 *  - DI-first (ServiceContainer, EventBus, CommandBus as references; no globals).
 *  - No threading in the base (no std::thread, no atomics) — zero overhead for cooperative agents.
 *  - Minimal state machine + guards; no exceptions (bool return, lastError string).
 *  - Dedicated-thread behavior will be provided by a separate BaseDedicatedAgent derived class.
 *
 * Typical usage (cooperative agent):
 * @code
 * class FileScanAgent final : public viz::core::BaseAgent {
 * public:
 *   using BaseAgent::BaseAgent; // inherit DI-ctor
 * protected:
 *   bool onStart() noexcept override { // allocate small buffers
 *     return true;
 *   }
 *   bool onTick(float dt) noexcept override {
 *     // do a small slice of work per frame
 *     return true;
 *   }
 *   void onStop() noexcept override { // flush/close
 *   }
 * };
 * @endcode
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <string>
#include <string_view>

#include "common/Attributes.hpp" // VIZ_NODISCARD, VIZ_LIKELY/UNLIKELY

namespace viz::core
{
    class ServiceContainer;
    class EventBus;
    class CommandBus;

    enum class AgentState : unsigned char
    {
        Constructed = 0,
        Initialized,
        Started,
        Stopped
    };

    class BaseAgent
    {
      public:
        BaseAgent(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept
            : m_services(services), m_events(events), m_commands(commands)
        {
        }

        virtual ~BaseAgent() = default;

        BaseAgent(const BaseAgent&) = delete;
        BaseAgent& operator=(const BaseAgent&) = delete;
        BaseAgent(BaseAgent&&) = delete;
        BaseAgent& operator=(BaseAgent&&) = delete;

        // ---------------- Lifecycle (guards) ----------------
        VIZ_NODISCARD bool initialize() noexcept
        {
            if (VIZ_UNLIKELY(m_state != AgentState::Constructed))
            {
                setError("initialize() requires Constructed state");
                return false;
            }
            // no-op in base
            clearError();
            m_state = AgentState::Initialized;
            return true;
        }

        VIZ_NODISCARD bool start() noexcept
        {
            if (VIZ_UNLIKELY(m_state != AgentState::Stopped && m_state != AgentState::Initialized))
            {
                setError("start() requires Initialized or Stopped state");
                return false;
            }
            if (!onStart())
            {
                setError("onStart() failed");
                return false;
            }
            m_state = AgentState::Started;
            clearError();
            return true;
        }

        VIZ_NODISCARD bool tick(float dt) noexcept
        {
            if (VIZ_UNLIKELY(m_state != AgentState::Started))
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

        VIZ_NODISCARD bool stop() noexcept
        {
            if (m_state == AgentState::Started)
            {
                onStop();
                m_state = AgentState::Stopped;
                clearError();
                return true;
            }
            setError("stop() requires Started state");
            return false;
        }

        VIZ_NODISCARD bool shutdown() noexcept
        {
            if (VIZ_UNLIKELY(m_state != AgentState::Stopped && m_state != AgentState::Initialized))
            {
                if (m_state == AgentState::Started && stop())
                {
                    // enforce stop before shutdown
                }
                else
                {
                    setError("shutdown() requires Stopped or Initialized state");
                    return false;
                }
            }
            // no-op in base
            m_state = AgentState::Constructed;
            clearError();
            return true;
        }

        /// @brief BaseAgent has no worker threads → join is a no-op (true).
        VIZ_NODISCARD virtual bool join() noexcept
        {
            return true;
        }

        // ---------------- Introspection ----------------
        VIZ_NODISCARD AgentState state() const noexcept
        {
            return m_state;
        }
        VIZ_NODISCARD bool isStarted() const noexcept
        {
            return m_state == AgentState::Started;
        }
        VIZ_NODISCARD bool isRunning() const noexcept
        {
            return isStarted();
        }
        VIZ_NODISCARD std::string const& lastError() const noexcept
        {
            return m_lastError;
        }

      protected:
        // -------- Hooks for derived agents --------
        virtual bool onStart() noexcept
        {
            return true;
        }
        virtual bool onTick(float /*dt*/) noexcept
        {
            return true;
        }
        virtual void onStop() noexcept {}

        // -------- DI accessors --------
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

        void setError(std::string_view msg) noexcept
        {
            m_lastError.assign(msg);
        }
        void clearError() noexcept
        {
            m_lastError.clear();
        }

      private:
        ServiceContainer& m_services;
        EventBus& m_events;
        CommandBus& m_commands;

        AgentState m_state{AgentState::Constructed};
        std::string m_lastError;
    };

} // namespace viz::core
