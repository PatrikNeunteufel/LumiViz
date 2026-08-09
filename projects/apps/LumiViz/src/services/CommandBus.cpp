/**
 ****************************************************************************************
 * @file   CommandBus.cpp
 * @brief  CommandBus implementation
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "services/CommandBus.hpp"
#include "services/IEventBus.hpp"
#include "services/events/CommandEvents.hpp"

namespace
{
    std::uint64_t steadyNowMs()
    {
        using namespace std::chrono;
        return static_cast<std::uint64_t>(
            duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }
} // namespace

CommandBus::CommandBus(IEventBus* eventBus,
                       std::size_t historyLimit,
                       std::uint64_t mergeWindowMs,
                       ClockFn clock)
    : m_eventBus(eventBus)
    , m_historyLimit(historyLimit > 0 ? historyLimit : 1)
    , m_mergeWindowMs(mergeWindowMs)
    , m_clock(clock ? std::move(clock) : ClockFn(&steadyNowMs))
{
}

bool CommandBus::execute(std::unique_ptr<ICommand> command)
{
    if (!command || !command->execute())
    {
        return false;  // no effect — nothing to record
    }

    // A new action invalidates the redo branch
    m_redoStack.clear();

    const std::uint64_t now = m_clock();

    // Drag coalescing: merge into the newest entry within the merge window
    if (!m_undoStack.empty()
        && (now - m_lastExecuteMs) <= m_mergeWindowMs
        && m_undoStack.back()->canMergeWith(*command))
    {
        m_undoStack.back()->mergeWith(*command);
    }
    else
    {
        m_undoStack.push_back(std::move(command));
        while (m_undoStack.size() > m_historyLimit)
        {
            m_undoStack.pop_front();
        }
    }

    m_lastExecuteMs = now;
    publishHistoryChanged(static_cast<int>(CommandHistoryChangedEvent::Cause::Executed));
    return true;
}

bool CommandBus::undo()
{
    if (m_undoStack.empty())
    {
        return false;
    }

    auto command = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    command->undo();
    m_redoStack.push_back(std::move(command));

    publishHistoryChanged(static_cast<int>(CommandHistoryChangedEvent::Cause::Undone));
    return true;
}

bool CommandBus::redo()
{
    if (m_redoStack.empty())
    {
        return false;
    }

    auto command = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    if (command->execute())
    {
        m_undoStack.push_back(std::move(command));
    }
    // else: the command no longer has an effect — drop it silently

    publishHistoryChanged(static_cast<int>(CommandHistoryChangedEvent::Cause::Redone));
    return true;
}

std::string CommandBus::undoDescription() const
{
    return m_undoStack.empty() ? std::string{} : m_undoStack.back()->description();
}

std::string CommandBus::redoDescription() const
{
    return m_redoStack.empty() ? std::string{} : m_redoStack.back()->description();
}

void CommandBus::clear()
{
    if (m_undoStack.empty() && m_redoStack.empty())
    {
        return;
    }
    m_undoStack.clear();
    m_redoStack.clear();
    publishHistoryChanged(static_cast<int>(CommandHistoryChangedEvent::Cause::Cleared));
}

void CommandBus::publishHistoryChanged(int cause) const
{
    if (!m_eventBus)
    {
        return;
    }

    CommandHistoryChangedEvent evt;
    evt.cause = static_cast<CommandHistoryChangedEvent::Cause>(cause);
    evt.canUndo = canUndo();
    evt.canRedo = canRedo();
    evt.undoDescription = undoDescription();
    evt.redoDescription = redoDescription();
    m_eventBus->publish(evt);
}
