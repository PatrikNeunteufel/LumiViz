/**
 ****************************************************************************************
 * @file   CommandDispatcher.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <string>

#include "CommandContext.hpp"
#include "CommandHistory.hpp"
#include "CommandRegistry.hpp"

namespace viz::core
{

    class CommandDispatcher
    {
      public:
        explicit CommandDispatcher(CommandRegistry& reg, CommandHistory& hist) : m_reg(reg), m_hist(hist) {}

        template<typename C>
        bool send(const C& cmd, const CommandContext& ctx, std::string label = {}, bool recordUndo = true);

      private:
        CommandRegistry& m_reg;
        CommandHistory& m_hist;
        static std::string defaultLabel(const char* name);
    };

} // namespace viz::core

#include "CommandDispatcher.tpp"
