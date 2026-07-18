/**
 ****************************************************************************************
 * @file   BaseCooperativeAgent.hpp
 * @brief  Cooperative agent variant running in the application's main loop via tick(dt).
 *
 * This class derives from BaseAgent and adds a single, semantically explicit hook:
 *   - onCooperativeTick(dt): preferred override point for cooperative work slices.
 *
 * Design:
 *  - No extra state or threads here (zero overhead).
 *  - The application (or a manager) calls tick(dt) while the agent is Started.
 *  - If you don't override onCooperativeTick(), the base's onTick(dt) path is used (no-op → true).
 *
 * For background workers, use BaseDedicatedAgent (separate file).
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include "BaseAgent.hpp"

namespace viz::core
{
    class BaseCooperativeAgent : public BaseAgent
    {
    public:
        using BaseAgent::BaseAgent; // inherit DI constructor

        virtual ~BaseCooperativeAgent() = default;

    protected:
        /**
         * @brief Preferred cooperative work hook. Override this in derived classes.
         * @param dt Delta time in seconds.
         * @return true on success; false to signal a soft failure to the caller.
         *
         * @note Keep the work slice small and deterministic. Do not block.
         */
        virtual bool onCooperativeTick(float dt) noexcept
        {
            // Default fallback to BaseAgent's generic onTick (which returns true by default).
            return BaseAgent::onTick(dt);
        }

        /**
         * @brief Bridge the BaseAgent tick to the cooperative hook.
         *
         * You normally don't override this; override onCooperativeTick(dt) instead for clarity.
         */
        bool onTick(float dt) noexcept final
        {
            return onCooperativeTick(dt);
        }
    };

} // namespace viz::core
