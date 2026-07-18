/**
 ****************************************************************************************
 * @file   CommandEvents.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <functional>

#include "ICommand.hpp"

namespace viz::core
{

    /**
     * @brief Lifecycle callbacks emitted by the CommandBus.
     */
    struct CommandEvents
    {
        std::function<void(const ICommand&)> willExecute{};                      ///< before execute()
        std::function<void(const ICommand&, const CommandResult&)> didExecute{}; ///< after execute()
        std::function<void(const ICommand&, const CommandResult&)> undone{};     ///< after undo()
        std::function<void(const ICommand&, const CommandResult&)> redone{};     ///< after redo()
    };

} // namespace viz::core
