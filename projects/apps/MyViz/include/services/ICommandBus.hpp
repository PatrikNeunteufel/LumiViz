/**
 ****************************************************************************************
 * @file   ICommandBus.hpp
 * @brief  Interface for the undo/redo command bus (Phase 4)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * Central undo/redo history. Commands are executed through the bus and land on
 * the undo stack; undo() reverts the newest, redo() re-applies it. Consecutive
 * mergeable commands within the merge window collapse into one undo step
 * (slider-drag coalescing). NOT thread-safe — main/UI thread only.
 ****************************************************************************************
 */

#pragma once

#include "ICommand.hpp"

#include <cstddef>
#include <memory>
#include <string>

class ICommandBus
{
public:
    virtual ~ICommandBus() = default;

    /**
     * @brief Execute a command and record it for undo
     * @return false if the command reported no effect (nothing recorded)
     */
    virtual bool execute(std::unique_ptr<ICommand> command) = 0;

    /// @brief Revert the newest command; false if history is empty
    virtual bool undo() = 0;

    /// @brief Re-apply the newest undone command; false if none
    virtual bool redo() = 0;

    [[nodiscard]] virtual bool canUndo() const = 0;
    [[nodiscard]] virtual bool canRedo() const = 0;

    /// @brief Description of the command undo()/redo() would affect ("" if none)
    [[nodiscard]] virtual std::string undoDescription() const = 0;
    [[nodiscard]] virtual std::string redoDescription() const = 0;

    [[nodiscard]] virtual std::size_t undoCount() const = 0;
    [[nodiscard]] virtual std::size_t redoCount() const = 0;

    /// @brief Drop the whole history (e.g. when the command targets die)
    virtual void clear() = 0;
};
