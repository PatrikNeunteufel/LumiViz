/**
 ****************************************************************************************
 * @file   CommandBus.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "CommandBus.hpp"

#include <cassert>
#include <chrono>

#include "core/eventbus/EventBus.hpp" // your existing templated EventBus
#include "CoalescingPolicy.hpp"
#include "CommandBusEvents.hpp"
#include "CompositeCommand.hpp"

namespace viz::core
{

    CommandBus::CommandBus() = default;
    CommandBus::CommandBus(EventBus* eb) : m_eventBus(eb) {}

    // --- helpers ---
    CommandResult CommandBus::checkThreadAffinity(const CommandContext& ctx)
    {
        if (ctx.mainThreadToken && ctx.currentThreadToken && ctx.currentThreadToken != ctx.mainThreadToken)
        {
            return CommandResult::Fail("Thread affinity violation", 1001);
        }
        return CommandResult::Ok();
    }

    void CommandBus::publishStacksChangedUnlocked()
    {
        if (!m_eventBus)
            return;
        m_eventBus->publish(CommandStacksChanged{!m_undo.empty(), !m_redo.empty(),
                                                 m_undo.empty() ? std::string{} : m_undo.back().label,
                                                 m_redo.empty() ? std::string{} : m_redo.back().label});
    }

    // --- submit ---
    CommandResult CommandBus::execute(const ICommandPtr& cmd, const CommandContext& ctx)
    {
        if (!cmd)
            return CommandResult::Fail("Null command");

        // Thread affinity check
        if (auto tf = checkThreadAffinity(ctx); !tf.ok)
            return tf;

        using clock = std::chrono::steady_clock;
        auto t0 = clock::now();

        std::lock_guard<std::mutex> lock(m_mtx);

        if (m_events.willExecute)
            m_events.willExecute(*cmd);
        if (m_eventBus)
            m_eventBus->publish(CommandWillExecute{cmd->name()});

        CommandResult res = cmd->execute(ctx);

        auto t1 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        if (m_events.didExecute)
            m_events.didExecute(*cmd, res);
        if (m_eventBus)
            m_eventBus->publish(CommandDidExecute{cmd->name(), res.ok, ms});

        if (!res.ok)
            return res;

        if (m_txDepth > 0)
        {
            m_txBuffer.push_back(cmd);
            // stacks unchanged (buffered), no publishStacksChanged
            return res;
        }

        // Coalescing: command-local OR global policy
        if (!m_undo.empty())
        {
            ICommandPtr& last = m_undo.back().cmd;
            if (last->tryMergeWith(cmd) || m_coalesce.tryMerge(*last, *cmd))
            {
                clearRedo();
                publishStacksChangedUnlocked();
                return res;
            }
        }

        clearRedo();
        m_undo.push_back(Entry{cmd, cmd->name()});
        publishStacksChangedUnlocked();
        return res;
    }

    CommandResult CommandBus::dispatch(const ICommandPtr& cmd, const CommandContext& ctx)
    {
        return execute(cmd, ctx);
    }

    // ---------------------------------------------
    // 3) submitAsync(...) – einfache Hintergrundausführung
    //    Achtung: Bus ist i. d. R. NICHT thread-safe → echtes Async
    //    sollte später über einen Executor/TaskQueue serialisiert werden.
    // ---------------------------------------------
    std::future<CommandResult> CommandBus::submitAsync(ICommandPtr cmd, CommandContext ctx,
                                                       std::function<void(const CommandResult&)> completion)
    {
        return std::async(
            std::launch::async,
            [this, cmd = std::move(cmd), ctx = std::move(ctx), completion = std::move(completion)]() mutable
            {
                // Achtung bzgl. Thread-Affinity (UI/Main-Thread) – falls nötig im execute() prüfen.
                CommandResult r = this->execute(cmd, ctx);
                if (completion)
                {
                    try
                    {
                        completion(r);
                    }
                    catch (...)
                    { /* swallow */
                    }
                }
                return r;
            });
    }

    // --- undo ---
    CommandResult CommandBus::undo(const CommandContext& ctx)
    {
        if (auto tf = checkThreadAffinity(ctx); !tf.ok)
            return tf;

        using clock = std::chrono::steady_clock;
        auto t0 = clock::now();

        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_txDepth > 0)
            return CommandResult::Fail("Cannot undo during transaction", 2);
        if (m_undo.empty())
            return CommandResult::Ok("Nothing to undo");

        Entry e = m_undo.back();
        m_undo.pop_back();

        CommandResult res = e.cmd->undo(ctx);

        auto t1 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        m_redo.push_back(e);

        if (m_events.undone)
            m_events.undone(*m_redo.back().cmd, res);
        if (m_eventBus)
        {
            m_eventBus->publish(CommandUndone{m_redo.back().label, res.ok, ms});
            publishStacksChangedUnlocked();
        }
        return res;
    }

    // --- redo ---
    CommandResult CommandBus::redo(const CommandContext& ctx)
    {
        if (auto tf = checkThreadAffinity(ctx); !tf.ok)
            return tf;

        using clock = std::chrono::steady_clock;
        auto t0 = clock::now();

        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_txDepth > 0)
            return CommandResult::Fail("Cannot redo during transaction", 3);
        if (m_redo.empty())
            return CommandResult::Ok("Nothing to redo");

        Entry e = m_redo.back();
        m_redo.pop_back();

        if (m_events.willExecute)
            m_events.willExecute(*e.cmd);
        if (m_eventBus)
            m_eventBus->publish(CommandWillExecute{e.label});

        CommandResult res = e.cmd->execute(ctx);

        auto t1 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        if (res.ok)
            m_undo.push_back(e);

        if (m_events.redone)
            m_events.redone(*m_undo.back().cmd, res);
        if (m_eventBus)
        {
            m_eventBus->publish(CommandRedone{m_undo.back().label, res.ok, ms});
            publishStacksChangedUnlocked();
        }
        return res;
    }

    // --- queries ---
    bool CommandBus::canUndo() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return !m_undo.empty();
    }
    bool CommandBus::canRedo() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return !m_redo.empty();
    }
    std::string CommandBus::topUndoName() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_undo.empty() ? std::string{} : m_undo.back().label;
    }
    std::string CommandBus::topRedoName() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_redo.empty() ? std::string{} : m_redo.back().label;
    }

    // --- transactions ---
    void CommandBus::beginTransaction(const std::string& label)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_txDepth++ == 0)
        {
            m_txLabel = label;
            m_txBuffer.clear();
        }
    }
    void CommandBus::commitTransaction()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        assert(m_txDepth > 0 && "commitTransaction without beginTransaction");
        if (--m_txDepth == 0)
        {
            if (!m_txBuffer.empty())
            {
                auto macro = std::make_shared<CompositeCommand>(m_txLabel.empty() ? "Transaction" : m_txLabel);
                for (auto& c : m_txBuffer)
                    macro->add(c);
                clearRedo();
                m_undo.push_back(Entry{macro, macro->name()});
                if (m_eventBus)
                    publishStacksChangedUnlocked();
            }
            m_txBuffer.clear();
            m_txLabel.clear();
        }
    }
    void CommandBus::rollbackTransaction(const CommandContext& ctx)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        assert(m_txDepth > 0 && "rollbackTransaction without beginTransaction");
        if (--m_txDepth == 0)
        {
            for (std::size_t i = m_txBuffer.size(); i-- > 0;)
                (void)m_txBuffer[i]->undo(ctx);
            m_txBuffer.clear();
            m_txLabel.clear();
            // Stacks unchanged (buffered only) → keine StacksChanged-Publikation
        }
    }

    void CommandBus::clearRedo()
    {
        m_redo.clear();
    }

    // --- snapshot hooks ---
    std::size_t CommandBus::undoSize() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_undo.size();
    }
    std::size_t CommandBus::redoSize() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_redo.size();
    }
    std::vector<std::string> CommandBus::undoLabels() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::vector<std::string> out;
        out.reserve(m_undo.size());
        for (auto const& e : m_undo)
            out.push_back(e.label);
        return out;
    }
    std::vector<std::string> CommandBus::redoLabels() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::vector<std::string> out;
        out.reserve(m_redo.size());
        for (auto const& e : m_redo)
            out.push_back(e.label);
        return out;
    }

} // namespace viz::core
