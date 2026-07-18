/**
 ****************************************************************************************
 * @file   TCommandAdapter.tpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <utility>

namespace viz::core
{

    template<typename C>
    TCommandAdapter<C>::TCommandAdapter(std::string label, C payload, DoFn doFn, UndoFn undoFn)
        : m_label(std::move(label)), m_payload(std::move(payload)), m_do(std::move(doFn)), m_undo(std::move(undoFn))
    {
        static_assert(DoFnC<DoFn, C>, "TCommandAdapter: DoFn must be CommandResult(const C&, const CommandContext&)");
        static_assert(UndoFnC<UndoFn, C>,
                      "TCommandAdapter: UndoFn must be CommandResult(const C&, const CommandContext&)");
    }

    template<typename C> const char* TCommandAdapter<C>::name() const noexcept
    {
        return m_label.c_str();
    }

    template<typename C> CommandResult TCommandAdapter<C>::execute(const CommandContext& ctx)
    {
        if (m_executed)
            return CommandResult::Ok();
        if (!m_undo || !m_do)
            return CommandResult::Fail("TCommandAdapter: missing Do/Undo function");
        CommandResult r = m_do(m_payload, ctx);
        if (r.ok)
            m_executed = true;
        return r;
    }

    template<typename C> CommandResult TCommandAdapter<C>::undo(const CommandContext& ctx)
    {
        if (!m_executed)
            return CommandResult::Ok(); // idempotent
        if (!m_undo)
            return CommandResult::Fail("TCommandAdapter: Undo function not set");
        CommandResult r = m_undo(m_payload, ctx);
        if (r.ok)
            m_executed = false;
        return r;
    }

} // namespace viz::core
