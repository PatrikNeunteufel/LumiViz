/**
 ****************************************************************************************
 * @file   ICommand.hpp
 * @brief  Command interface for undo/redo (Phase 4, portiert aus harvest/core-module)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <string>

/**
 * @class ICommand
 * @brief A reversible operation for the CommandBus
 *
 * execute() performs the operation (also used for redo), undo() reverts it.
 * Coalescing: consecutive commands of the same kind (e.g. slider drags on the
 * same parameter) can be merged into one undo step via canMergeWith/mergeWith.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /**
     * @brief Perform (or re-perform) the operation
     * @return false if the operation had no effect — the bus will not record it
     */
    virtual bool execute() = 0;

    /// @brief Revert the operation
    virtual void undo() = 0;

    /// @brief Human-readable description (for menu texts / logging)
    [[nodiscard]] virtual std::string description() const = 0;

    /**
     * @brief May @p next be merged into this command (same target)?
     *
     * Only consulted for the newest undo-stack entry within the bus's
     * merge window (slider-drag coalescing).
     */
    [[nodiscard]] virtual bool canMergeWith(const ICommand& next) const
    {
        (void)next;
        return false;
    }

    /// @brief Adopt the target state of @p next (old state stays from *this)
    virtual void mergeWith(const ICommand& next) { (void)next; }
};
