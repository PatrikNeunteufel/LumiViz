/**
 ****************************************************************************************
 * @file   CommandBus.hpp
 * @brief  CommandBus implementation — undo/redo history with drag coalescing
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "ICommandBus.hpp"

#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>

class IEventBus;

/**
 * @class CommandBus
 * @brief Undo/redo history (see ICommandBus)
 *
 * - Publishes CommandHistoryChangedEvent on the optional IEventBus.
 * - Merge window: a command that canMergeWith() the newest undo entry and
 *   arrives within @p mergeWindowMs is merged into it (one undo step per
 *   slider drag instead of dozens).
 * - History limit: oldest entries are dropped beyond @p historyLimit.
 * - NOT thread-safe — main/UI thread only.
 */
class CommandBus : public ICommandBus
{
public:
    using ClockFn = std::function<std::uint64_t()>;  ///< Monotonic milliseconds

    explicit CommandBus(IEventBus* eventBus = nullptr,
                        std::size_t historyLimit = 100,
                        std::uint64_t mergeWindowMs = 750,
                        ClockFn clock = {});

    ~CommandBus() override = default;

    CommandBus(const CommandBus&) = delete;
    CommandBus& operator=(const CommandBus&) = delete;

    // ICommandBus
    bool execute(std::unique_ptr<ICommand> command) override;
    bool undo() override;
    bool redo() override;
    [[nodiscard]] bool canUndo() const override { return !m_undoStack.empty(); }
    [[nodiscard]] bool canRedo() const override { return !m_redoStack.empty(); }
    [[nodiscard]] std::string undoDescription() const override;
    [[nodiscard]] std::string redoDescription() const override;
    [[nodiscard]] std::size_t undoCount() const override { return m_undoStack.size(); }
    [[nodiscard]] std::size_t redoCount() const override { return m_redoStack.size(); }
    void clear() override;

private:
    enum class Cause : int;  // maps to CommandHistoryChangedEvent::Cause

    void publishHistoryChanged(int cause) const;

    IEventBus* m_eventBus = nullptr;
    std::size_t m_historyLimit;
    std::uint64_t m_mergeWindowMs;
    ClockFn m_clock;

    std::deque<std::unique_ptr<ICommand>> m_undoStack;  ///< back = newest
    std::deque<std::unique_ptr<ICommand>> m_redoStack;  ///< back = next redo
    std::uint64_t m_lastExecuteMs = 0;
};
