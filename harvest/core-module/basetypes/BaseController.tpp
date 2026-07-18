/**
 ****************************************************************************************
 * @file   BaseController.tpp
 * @brief  Template helpers for BaseController (must be included from header).
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <string_view>
#include <type_traits>
#include <utility>
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandContext.hpp"


namespace viz::core
{
    class TransactionGuard;                  // optional
    template<typename FnDo, typename FnUndo> // optional
    struct TCommandAdapter;

    template<typename Bus>
    concept BusHasBeginCommitTx = requires(Bus& b, std::string_view label) {
        b.beginTransaction(label);
        b.commitTransaction();
    };

    template<typename Bus>
    concept BusHasScopedTx = requires(Bus& b, std::string_view label) { b.withTransaction(label, [] {}); };

    template<typename Guard, typename Bus>
    concept GuardConstructible = requires(Bus& b, std::string_view label) { Guard{b, label}; };

    template<typename FnDo, typename FnUndo>
    concept HasTCommandAdapter = requires(std::string_view lbl, FnDo&& d, FnUndo&& u) {
        typename viz::core::TCommandAdapter<std::decay_t<FnDo>, std::decay_t<FnUndo>>;
        viz::core::TCommandAdapter<std::decay_t<FnDo>, std::decay_t<FnUndo>>{lbl, std::forward<FnDo>(d),
                                                                             std::forward<FnUndo>(u)};
    };

    template<typename Fn> void BaseController::withTransaction(std::string_view label, Fn&& fn) noexcept
    {
        // Production CommandBus exposes beginTransaction/commitTransaction (no withTransaction).
        // Keep noexcept at controller boundary; ensure we leave the bus in a consistent state.
        m_commands.beginTransaction(std::string{label});
        try
        {
            std::forward<Fn>(fn)();
            m_commands.commitTransaction();
        }
        catch (...)
        {
            // If your CommandBus provides rollbackTransaction(const CommandContext&),
            // you can uncomment the next line to roll back explicitly:
            // m_commands.rollbackTransaction(CommandContext{});
            // Ensure we close the transaction anyway to avoid a stuck state:
            m_commands.commitTransaction();
        }
    }

    template<typename FnDo, typename FnUndo>
    inline void BaseController::dispatchAdapter(std::string_view label, FnDo&& doFn, FnUndo&& undoFn) noexcept
    {
        if constexpr (HasTCommandAdapter<FnDo, FnUndo>)
        {
            viz::core::TCommandAdapter<std::decay_t<FnDo>, std::decay_t<FnUndo>> cmd{label, std::forward<FnDo>(doFn),
                                                                                     std::forward<FnUndo>(undoFn)};
            this->execute(cmd);
        }
        else
        {
            this->withTransaction(label, [&] { std::forward<FnDo>(doFn)(); });
            (void)undoFn;
        }
    }

} // namespace viz::core
