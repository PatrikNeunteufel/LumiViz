/**
 ****************************************************************************************
 * @file   ICommand.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <memory>

#include "CommandContext.hpp"
#include "CommandResult.hpp"

namespace viz::core
{

    /**
     * @brief Base interface for every executable, undoable action.
     * - execute(): apply the state change; must be exception-safe.
     * - undo(): revert the last execute(); must be idempotent.
     * - tryMergeWith(): enable coalescing of small sequential edits.
     */
    class ICommand
    {
      public:
        virtual ~ICommand() = default;

        virtual const char* name() const noexcept = 0;
        virtual CommandResult execute(const CommandContext& ctx) = 0;
        virtual CommandResult undo(const CommandContext& ctx) = 0;
        virtual bool tryMergeWith(const std::shared_ptr<ICommand>& /*next*/)
        {
            return false;
        }
    };

    using ICommandPtr = std::shared_ptr<ICommand>;

} // namespace viz::core
