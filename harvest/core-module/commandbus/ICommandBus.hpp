/**
 ****************************************************************************************
 * @file   ICommandBus.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include "core/commandbus/ICommand.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
#include "core/commandbus/CommandEvents.hpp"
#include <functional>
#include <future>
#include <string>

namespace viz::core
{

    /**
     * @brief Interface for the command bus (submit/undo/redo, transactions, events).
     */
    class ICommandBus
    {
      public:
        virtual ~ICommandBus() = default;

        /// Synchronous execution (formerly `submit`): runs now, updates undo/redo.
        virtual CommandResult execute(const ICommandPtr& cmd, const CommandContext& ctx = {}) = 0;

        /// Semantic alias for controllers. Same behavior as `execute`.
        virtual CommandResult dispatch(const ICommandPtr& cmd, const CommandContext& ctx = {})
    {
        return execute(cmd, ctx);
    }

        /// Asynchronous submission. Runs off-thread, returns future and/or invokes completion.
        virtual std::future<CommandResult> submitAsync(ICommandPtr cmd, CommandContext ctx = {},
                                                       std::function<void(const CommandResult&)> completion = {}) = 0;


        virtual CommandResult undo(const CommandContext& ctx) = 0;
        virtual CommandResult redo(const CommandContext& ctx) = 0;

        virtual bool canUndo() const noexcept = 0;
        virtual bool canRedo() const noexcept = 0;
        virtual std::string topUndoName() const = 0;
        virtual std::string topRedoName() const = 0;

        virtual void beginTransaction(const std::string& label = {}) = 0;
        virtual void commitTransaction() = 0;
        virtual void rollbackTransaction(const CommandContext& ctx) = 0;

        virtual CommandEvents& events() = 0;
    };

} // namespace viz::core
