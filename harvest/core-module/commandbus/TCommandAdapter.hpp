/**
 ****************************************************************************************
 * @file   TCommandAdapter.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <functional>
#include <string>
#include <type_traits>

#include "ICommand.hpp"

namespace viz::core
{
    // ---- C++20 Concepts (global, wiederverwendbar) --------------------------------

    template<class F, class C>
    concept DoFnC = std::is_invocable_r_v<CommandResult, F, const C&, const CommandContext&>;
    template<class F, class C>
    concept UndoFnC = std::is_invocable_r_v<CommandResult, F, const C&, const CommandContext&>;

     /**
     * @brief Template adapter turning a payload type C and two callables into an ICommand.
     *
     * Usage idea:
     *   struct SetValue { int id; int value; };
     *   auto cmd = std::make_shared<TCommandAdapter<SetValue>>(
     *       "SetValue",
     *       SetValue{42, 7},
     *       // Do
     *       [&](const SetValue& p, const CommandContext& ctx) -> CommandResult { ... },
     *       // Undo
     *       [&](const SetValue& p, const CommandContext& ctx) -> CommandResult { ... }
     *   );
     *   bus.submit(cmd, ctx);
     *
     * Contract:
     *  - DoFn:   CommandResult(const C&, const CommandContext&)
     *  - UndoFn: CommandResult(const C&, const CommandContext&)
     *  - undo() must be idempotent — adapter enforces "no-op" if not executed yet.
     */
    template<typename C> class TCommandAdapter final : public ICommand
    {
      public:
        using DoFn = std::function<CommandResult(const C&, const CommandContext&)>;
        using UndoFn = std::function<CommandResult(const C&, const CommandContext&)>;

        TCommandAdapter(std::string label, C payload, DoFn doFn, UndoFn undoFn);

        const char* name() const noexcept override;

        CommandResult execute(const CommandContext& ctx) override;
        CommandResult undo(const CommandContext& ctx) override;

      private:
        std::string m_label;
        C m_payload;
        DoFn m_do;
        UndoFn m_undo;
        bool m_executed{false};
    };

} // namespace viz::core

#include "TCommandAdapter.tpp"

