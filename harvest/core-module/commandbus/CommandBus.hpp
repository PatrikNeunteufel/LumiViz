/**
 ****************************************************************************************
 * @file   CommandBus.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <mutex>
#include <string>
#include <vector>

#include "ICommandBus.hpp"
#include "CoalescingPolicy.hpp"     // NEW
#include "CommandBusEvents.hpp"     // for StacksChanged payload type


namespace viz::core
{

    class EventBus; // fwd (real include nur in .cpp)

    class CommandBus final : public ICommandBus
    {
      public:
        CommandBus();
        explicit CommandBus(EventBus* eb); ///< enable bridge
        ~CommandBus() override = default;

        void setEventBus(EventBus& eb)
        {
            m_eventBus = &eb;
        }

        // ICommandBus:
        CommandResult execute(const ICommandPtr& cmd, const CommandContext& ctx) override;

        // 2) optional: expliziter Alias (Interface delegiert bereits auf execute)
        CommandResult dispatch(const ICommandPtr& cmd, const CommandContext& ctx = {}) override;

        // 3) asynchron: sauber getrennt
        std::future<CommandResult> submitAsync(ICommandPtr cmd, CommandContext ctx = {},
                                               std::function<void(const CommandResult&)> completion = {}) override;

        CommandResult undo(const CommandContext& ctx) override;
        CommandResult redo(const CommandContext& ctx) override;

        bool canUndo() const noexcept override;
        bool canRedo() const noexcept override;
        std::string topUndoName() const override;
        std::string topRedoName() const override;

        void beginTransaction(const std::string& label = {}) override;
        void commitTransaction() override;
        void rollbackTransaction(const CommandContext& ctx) override;

        CommandEvents& events() override
        {
            return m_events;
        }

        // ---- History snapshot hooks (NEW) ----
        std::size_t undoSize() const noexcept;
        std::size_t redoSize() const noexcept;
        std::vector<std::string> undoLabels() const;
        std::vector<std::string> redoLabels() const;

        // ---- Coalescing Policy (NEW) ----
        CoalescingPolicy& coalescing() noexcept
        {
            return m_coalesce;
        }
        const CoalescingPolicy& coalescing() const noexcept
        {
            return m_coalesce;
        }

      private:
        struct Entry
        {
            ICommandPtr cmd;
            std::string label;
        };

        void clearRedo();
        void publishStacksChangedUnlocked();                                 // NEW: assumes m_mtx is held
        static CommandResult checkThreadAffinity(const CommandContext& ctx); // NEW

        mutable std::mutex m_mtx;
        std::vector<Entry> m_undo;
        std::vector<Entry> m_redo;

        int m_txDepth{0};
        std::vector<ICommandPtr> m_txBuffer;
        std::string m_txLabel;

        CommandEvents m_events;
        EventBus* m_eventBus{nullptr}; // bridge

        CoalescingPolicy m_coalesce; // NEW
    };

} // namespace viz::core
