/**
 ****************************************************************************************
 * @file   BaseDedicatedAgent.hpp
 * @brief  Dedicated-thread agent variant: encapsulates a worker thread and a simple stop/join contract.
 *
 * Concept:
 *   BaseAgent (no threads)  →  BaseDedicatedAgent (owns std::thread)
 *
 * Lifecycle:
 *   start()  -> spawns worker thread (calls onThreadStart() once)
 *            -> loop: while !stopRequested: onThreadTick(); waitNextCycle();
 *            -> before exit: onThreadStop()
 *   stop()   -> sets stopRequested flag (non-blocking)
 *   join()   -> joins worker thread (blocking)
 *
 * Usage:
 * @code
 * class FileWatcherAgent final : public viz::core::BaseDedicatedAgent {
 * public:
 *   using BaseDedicatedAgent::BaseDedicatedAgent;
 * protected:
 *   bool onThreadStart() noexcept override { // open handles
 *     return true;
 *   }
 *   bool onThreadTick() noexcept override {
 *     // do a small chunk of work; return true to continue
 *     return true;
 *   }
 *   void onThreadStop() noexcept override { // close handles
 *   }
 * };
 *
 * // Orchestration:
 * // watcher.start();
 * // ...
 * // watcher.stop();   // request stop
 * // watcher.join();   // synchronize termination
 * @endcode
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

#include "BaseAgent.hpp"

namespace viz::core
{
    class BaseDedicatedAgent : public BaseAgent
    {
      public:
        using BaseAgent::BaseAgent;

        void setTickInterval(std::chrono::nanoseconds interval) noexcept
        {
            m_tickInterval = interval;
        }

        VIZ_NODISCARD bool stopRequested() const noexcept
        {
            return m_stopRequested.load(std::memory_order_acquire);
        }

        VIZ_NODISCARD bool join() noexcept override
        {
            if (m_thread.joinable())
                m_thread.join();
            return true;
        }

        ~BaseDedicatedAgent() override
        {
#if !defined(NDEBUG)
            if (isStarted())
            {
                requestStop(); // no join here
            }
#endif
        }

      protected:
        // ---- wire worker to BaseAgent lifecycle ----
        bool onStart() noexcept override
        {
            if (m_running.load(std::memory_order_acquire))
                return false;

            m_stopRequested.store(false, std::memory_order_release);
            m_running.store(true, std::memory_order_release);

            try
            {
                m_thread = std::thread(&BaseDedicatedAgent::threadMain, this);
            }
            catch (...)
            {
                m_running.store(false, std::memory_order_release);
                return false;
            }
            return true;
        }

        void onStop() noexcept override
        {
            requestStop(); // ask thread to exit; caller must call join()
        }

        bool onTick(float /*dt*/) noexcept override
        {
            return true;
        }

        // ---- extension points ----
        virtual bool onThreadStart() noexcept
        {
            return true;
        }
        virtual bool onThreadTick() noexcept
        {
            return true;
        }
        virtual void onThreadStop() noexcept {}

        virtual void waitNextCycle() noexcept
        {
            auto const ns = m_tickInterval;
            if (ns.count() > 0)
                std::this_thread::sleep_for(ns);
            else
                std::this_thread::yield();
        }

        void requestStop() noexcept
        {
            m_stopRequested.store(true, std::memory_order_release);
        }

      private:
        void threadMain() noexcept
        {
            if (!onThreadStart())
            {
                m_running.store(false, std::memory_order_release);
                return;
            }

            while (!m_stopRequested.load(std::memory_order_acquire))
            {
                if (!onThreadTick())
                    break;
                waitNextCycle();
            }

            onThreadStop();
            m_running.store(false, std::memory_order_release);
        }

      private:
        std::thread m_thread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_stopRequested{false};
        std::chrono::nanoseconds m_tickInterval{std::chrono::nanoseconds{0}};
    };

} // namespace viz::core
